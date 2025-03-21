<?php
/**
 * @file
 * Provides an admin interface for managing recipes.
 */

/**
 * Menu callback: Admin form & listing.
 */
function recipes_admin($form, &$form_state = array()) {

  // Get current list of modules.
  $files = system_rebuild_module_data();

  $visible_files = $files;
  foreach ($visible_files as $filename => $file) {
    // Remove non-recipes from the list.
    if (!isset($file->info['package']) || $file->info['package'] != 'Recipes') {
      unset($visible_files[$filename]);
    }
  }

  $modules = array();
  $form['modules'] = array('#tree' => TRUE);

  // Used when displaying modules that are required by the installation profile.
  require_once BACKDROP_ROOT . '/core/includes/install.inc';
  $distribution_name = check_plain(backdrop_install_profile_distribution_name());

  // Iterate through each of the modules.
  foreach ($visible_files as $filename => $module) {
    $extra = array();
    $extra['enabled'] = (bool) $module->status;
    $extra['disabled'] = (array_key_exists('disabled', $module->info)) ? $module->info['disabled'] : FALSE;
    if ($extra['disabled'] && !empty($module->info['explanation'])) {
      $extra['required_by'][] = $module->info['explanation'];
    }

    // Some modules are always required by core and cannot be disabled, for
    // example: entity, node, system, and user.
    $extra['required_by_distribution'] = FALSE;
    if (!empty($module->info['required'])) {
      $extra['disabled'] = TRUE;
      $extra['required_by'][] = $distribution_name . (!empty($module->info['explanation']) ? ' (' . $module->info['explanation'] . ')' : '');
      $extra['required_by_distribution'] = TRUE;
    }

    // If this module requires other modules, add them to the array.
    foreach ($module->requires as $requires => $v) {
      if (!isset($files[$requires])) {
        $extra['requires'][$requires] = t('@module (<span class="admin-missing">missing</span>)', array('@module' => backdrop_ucfirst($requires)));
        $extra['disabled'] = TRUE;
      }
      // Only display visible modules.
      elseif (isset($visible_files[$requires])) {
        $requires_name = $files[$requires]->info['name'];

        // Disable this module if it is incompatible with the dependency's version.
        $version_string = FALSE;
        // Make sure the version string of the dependency is available.
        if (!empty($files[$requires]->info['version'])) {
          $version_string = preg_replace('/^' . preg_quote(BACKDROP_CORE_COMPATIBILITY, '/') . '-/', '', $files[$requires]->info['version']);
        }
        if ($version_string && $incompatible_version = backdrop_check_incompatibility($v, $version_string)) {
          $extra['requires'][$requires] = t('@module (<span class="admin-missing">incompatible with</span> version @version)', array(
            '@module' => $requires_name . ' ' . $incompatible_version,
            '@version' => $version_string,
          ));
          $extra['disabled'] = TRUE;
        }

        // Disable this module if the dependency is incompatible with this version of Backdrop core.
        elseif (isset($files[$requires]->info['backdrop']) && $files[$requires]->info['backdrop'] != BACKDROP_CORE_COMPATIBILITY) {
          $extra['requires'][$requires] = t('@module (<span class="admin-missing">incompatible with</span> this version of Backdrop core)', array(
            '@module' => $requires_name,
          ));
          $extra['disabled'] = TRUE;
        }
        elseif ($files[$requires]->status) {
          $extra['requires'][$requires] = t('@module (<span class="admin-enabled">enabled</span>)', array('@module' => $requires_name));
        }
        else {
          $extra['requires'][$requires] = t('@module (<span class="admin-disabled">disabled</span>)', array('@module' => $requires_name));
        }
      }
    }

    // If this module is required by other modules, list those, and then make it
    // impossible to disable this one.
    foreach ($module->required_by as $required_by => $v) {
      // Hidden modules are unset already.
      if (isset($visible_files[$required_by])) {
        if ($files[$required_by]->status == 1 && $module->status == 1) {
          $extra['required_by'][] = t('@module (<span class="admin-enabled">enabled</span>)', array('@module' => $files[$required_by]->info['name']));
          $extra['disabled'] = TRUE;
        }
        else {
          $extra['required_by'][] = t('@module (<span class="admin-disabled">disabled</span>)', array('@module' => $files[$required_by]->info['name']));
        }
      }
    }
    $form['modules'][$module->info['package']][$filename] = _recipes_build_row($module->info, $extra);
  }

  // Add basic information to the fieldsets.
  foreach (element_children($form['modules']) as $package) {
    $form['modules'][$package] += array(
      '#type' => 'fieldset',
      '#title' => t($package),
      '#theme' => 'recipe_fieldset',
      '#header' => array(
        t('Name'),
        array('data' => t('Version'), 'class' => array(RESPONSIVE_PRIORITY_MEDIUM)),
        array('data' => t('Description'), 'class' => array(RESPONSIVE_PRIORITY_LOW)),
        array('data' => t('Requirements'), 'class' => array(RESPONSIVE_PRIORITY_LOW)),
        array('data' => t('Operations')),
      ),
    );
  }
  ksort($form['modules']);

  return $form;
}

/**
 * Helper function: Builds a row in the recipes list.
 */
function _recipes_build_row($info, $extra) {
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
      if ($extra['disabled'] && $action_key == 'import') {
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
    $path = 'admin/structure/recipes/' . $action_key . '/' . $info['project'];
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

