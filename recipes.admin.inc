<?php
/**
 * @file
 * Provides an admin interface for managing recipes.
 */

/**
 * Menu callback: Admin form & listing.
 */
function recipes_admin_listing($form, &$form_state = array()) {
  // JS-only table filters.
  $form['filter'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('table-filter', 'js-show'),
    ),
    '#attached' => array(
      'library' => array(array('recipes', 'recipes.admin')),
    ),
  );
  $form['filter']['search'] = array(
    '#type' => 'textfield',
    '#title' => t('Filter'),
    '#size' => 30,
    '#placeholder' => t('Search...'),
    '#attributes' => array(
      'class' => array('table-filter-text'),
      'autocomplete' => 'off',
      'title' => t('Enter a part of the recipe name or description to filter.'),
    ),
    '#default_value' => (isset($_GET['search'])) ? $_GET['search'] : '',
  );
  $form['filter']['reset'] = array(
    '#theme' => 'link',
    '#text' => t('Reset'),
    '#path' => 'admin/structure/recipes',
    '#options' => array(
      'attributes' => array(
        'class' => array('button', 'button-secondary', 'search-reset'),
      ),
    ),
  );

  $form['recipes'] = array('#tree' => TRUE);

  $system_info = _recipes_get_system_info(TRUE);
  $recipes = recipes_load_multiple();

  foreach ($recipes as $signature => $recipe) {
    $recipe_status = recipes_get_status($signature);
    $messages = (!empty($recipe_status['messages'])) ? ' <div class="requirements admin-requirements">' . implode('<br />', $recipe_status['messages']) . '</div>' : '';
    // Build a requirements toggle.
    $show_toggle_statuses = array(RECIPES_STATUS_UNAVAILABLE, RECIPES_STATUS_CHANGED);
    $toggle = in_array($recipe_status['key'], $show_toggle_statuses) ? ' <a class="requirements-toggle" href="#">' . t('more') . '<span class="arrow"></span></a>' : '';
    // $status = recipes_get_status($signature);
    $status = ' <div class="recipe-status recipe-status--' . $recipe_status['key'] . '">' . $recipe_status['key'] . $toggle . '</div>' . $messages;
    $group = $recipe['group'] ?? t('General');
    $form['recipes'][$group][$signature] = array(
      '#name' => theme('label_machine_name__recipe', array(
        'label' => $recipe['label'] ?? $recipe['name'],
        'machine_name' => $recipe['signature'],
      )),
      '#description' => $recipe['description'] ?? t('No description available.'),
      '#status' => $status,
      '#status-key' => $recipe_status['key'] ?? '',
      'operations' => recipes_admin_operations($recipe),
    );
  }

  foreach (element_children($form['recipes']) as $group) {
    $form['recipes'][$group] += array(
      '#type' => 'fieldset',
      '#title' => $group,
      '#collapsible' => TRUE,
      '#theme' => 'recipes_fieldset',
    );
  }
  ksort($form['recipes']);

  return $form;
}

  /**
   * Helper function: Builds a row in the recipes list.
   */
function _recipes_build_simple_recipes_row($info, $extra) {
  // Prepare the tag list by ensuring all tags are lower case.
  $tags = array();
  if (isset($info['tags'])) {
    foreach ($info['tags'] as $tag) {
      $tags[] = check_plain($tag);
    }
  }
  if (isset($info['package'])) {
    $tags[] = check_plain($info['package']);
  }
  if (empty($tags)) {
    $tags[] = 'Other';
  }

  // Add in the defaults.
  $extra += array(
    'requires' => array(),
    'required_by' => array(),
    'disabled' => FALSE,
    'enabled' => FALSE,
    'links' => array(),
  );
  $form = array(
    '#tree' => TRUE,
  );
  // Set the basic properties.
  $form['name'] = array(
    '#markup' => $info['name'],
  );
  $form['description'] = array(
    '#markup' => t($info['description']),
  );
  $form['version'] = array(
    '#markup' => $info['version'],
  );
  $form['tags'] = array(
    '#markup' => implode(', ', $tags),
  );

  $form['#requires'] = $extra['requires'];
  $form['#required_by'] = $extra['required_by'];
  $form['#required_by_distribution'] = $extra['required_by_distribution'];
  if (!empty($info['project'])) {
    $form['#project'] = $info['project'];
  }

  // Check the compatibilities.
  $compatible = TRUE;
  $status_short = '';
  $status_long = '';

  // Check the core compatibility.
  if (!isset($info['backdrop']) || $info['backdrop'] != BACKDROP_CORE_COMPATIBILITY) {
    $compatible = FALSE;
    $status_short .= t('Incompatible with Backdrop CMS !core_version.', array('!core_version' => BACKDROP_CORE_COMPATIBILITY)) . ' ';
    $status_long .= t('This version is not compatible with Backdrop CMS !core_version.', array('!core_version' => BACKDROP_CORE_COMPATIBILITY)) . ' ';
  }

  // Ensure this module is compatible with the currently installed version of PHP.
  if (version_compare(phpversion(), $info['php']) < 0) {
    $compatible = FALSE;
    $status_short .= t('Incompatible with this version of PHP') . ' ';
    $php_required = $info['php'];
    if (substr_count($info['php'], '.') < 2) {
      $php_required .= '.*';
    }
    $status_long .= t('This module requires PHP version @php_required and is incompatible with PHP version !php_version.', array('@php_required' => $php_required, '!php_version' => phpversion())) . ' ';
  }

  $operations = array();
  // If this module is compatible, present a list of available operations.
  if ($compatible) {
    $actions = recipes_actions_get_info();
    foreach ($actions as $action_key => $action_info) {
      $requirements = TRUE;
      $requirements_callback = $action_info['requirements_callback'];
      if (function_exists($requirements_callback)) {
        $requirements = $requirements_callback($info['project']);
      }
      if (!$requirements) {
        continue;
      }
      if ($extra['disabled'] && $action_key == 'apply') {
        continue;
      }
      if (!empty($action_info['permission'])) {
        foreach ($action_info['permission'] as $permission) {
          if (user_access($permission)) {
            $operations[$action_key] = $action_info;
          }
        }
      }
    }
  }
  else {
    $form['description']['#markup'] .= theme('system_modules_incompatible',
    array('message' => trim($status_long)));
  }

  // Build dropbutton links.
  $links = array();
  foreach ($operations as $action_key => $action_info) {
    $path = 'admin/structure/recipes/' . $action_key . '/' . $recipe['signature'];
    $links[$action_key] = array(
      'title' => $action_info['label'],
      'href' => $path,
      'query' => array(
        'destination' => 'admin/structure/recipes',
        'token' => backdrop_get_token($path),
      ),
    );
  }
  if (!empty($links)) {
    $form['links'] = array(
      '#type' => 'dropbutton',
      '#links' => $links,
    );
  }
  else {
    $form['links'] = array(
      '#type' => 'markup',
      '#markup' => t('Disabled'),
    );
  }

  return $form;
}

  /**
  * Helper function to build the operations dropbutton for a recipe.
  *
  * @param array $recipe
  *   The recipe implementation array.
  *
  * @return array
  *   The renderable form array for the operations dropbutton.
  */
function recipes_admin_operations($recipe) {
  $recipe_status = recipes_get_status($recipe['signature']);
  $operations = array();
  $actions = recipes_actions_get_info();
  foreach ($actions as $action_key => $action_info) {
    $requirements = TRUE;
    $requirements_callback = $action_info['requirements_callback'];
    if (function_exists($requirements_callback)) {
      $requirements = $requirements_callback($recipe);
    }
    if (!$requirements) {
      continue;
    }
    if (in_array($recipe_status['key'], array(RECIPES_STATUS_UNAVAILABLE, RECIPES_STATUS_INSTALLED)) && $action_key == 'apply') {
      continue;
    }
    if ($recipe_status['key'] == RECIPES_STATUS_CHANGED && $action_key == 'apply') {
      $action_info['label'] = t('Reapply');
    }
    if (!empty($action_info['permission'])) {
      foreach ($action_info['permission'] as $permission) {
        if (user_access($permission)) {
          $operations[$action_key] = $action_info;
        }
      }
    }
  }

  $links = array();
  foreach ($operations as $action_key => $action_info) {
    $path = 'admin/structure/recipes/' . $action_key . '/' . $recipe['signature'];
    $links[$action_key] = array(
      'title' => $action_info['label'],
      'href' => $path,
      'query' => array(
        'destination' => 'admin/structure/recipes',
        'token' => backdrop_get_token($path),
      ),
    );
  }

  if (!empty($links)) {
    return array(
      '#type' => 'dropbutton',
      '#links' => $links,
    );
  }
  else {
    return array(
      '#type' => 'markup',
      '#markup' => t('Disabled'),
    );
  }
}

/**
 * Shows diff of specified configuration file.
 *
 * @param string $config_file
 *   The name of the configuration file.
 *
 * @return string
 *   Table showing a two-way diff between the active and staged configuration.
 */
function recipes_config_diff_page($recipe_signature, $config_file) {
  $diff = recipes_config_diff($recipe_signature, $config_file);

  $build = array();

  $build['#title'] = t('View changes of @config_file for @recipe', array('@config_file' => $config_file, '@recipe' => $recipe_signature));
  // Add the CSS for the inline diff.
  $build['#attached']['css'][] = backdrop_get_path('module', 'system') . '/css/system.diff.css';

  $build['diff'] = array(
    '#theme' => 'table',
    '#header' => array(
      array('data' => t('Site'), 'colspan' => '2', 'class' => array('config-old')),
      array('data' => t('Recipe'), 'colspan' => '2', 'class' => array('config-new')),
    ),
    '#attributes' => array('class' => array('diff-table')),
    '#rows' => $diff,
    '#sticky' => FALSE,
  );

  $build['actions'] = array(
    '#type' => 'actions',
    '#attributes' => array('class' => array('form-actions')),
  );
  $build['actions']['back'] = array(
    '#type' => 'link',
    '#title' => t('Back to "Recipes" page.'),
    '#href' => 'admin/structure/recipes',
    '#attributes' => array('class' => array('button', 'button-secondary', 'form-submit')),
  );

  if (backdrop_is_dialog()) {
    $build['actions']['back']['#attributes']['class'][] = 'dialog-cancel';
    $build['actions']['back']['#title'] = t('Close');
  }

  return $build;
}

/**
 * Return a formatted diff of a named config between staging and active.
 *
 * @param string $recipe_signature
 *   The unique signature of the recipe to which the configuration belongs.
 * @param string $name
 *   The name of the configuration object to diff.
 *
 * @return array
 *   An array of formatted strings showing the diffs between the two storages.
 *
 * @see recipes_config_diff_page()
 */
function recipes_config_diff($recipe_signature, $name) {
  $active_storage = config_get_config_storage('active');
  $recipe_configs = _recipes_do('get', 'config', $recipe_signature);
  $recipe_storage = new ConfigFileStorage($recipe_configs['path']);

  $source_data = explode("\n", backdrop_json_encode($active_storage->read($name), TRUE));
  $target_data = explode("\n", backdrop_json_encode($recipe_storage->read($name), TRUE));

  // Check for new or removed config.
  if ($source_data === array('false')) {
    // Added file.
    $source_data = array(t('Config added'));
  }
  if ($target_data === array('false')) {
    // Deleted file.
    $target_data = array(t('Config removed'));
  }

  $diff = new Diff($source_data, $target_data);
  $formatter = new BackdropDiffFormatter();
  return $formatter->format($diff);
}
