<?php
/**
 * @file
 * Recipe actions.
 */

/**
 * Form: Applies a recipe to a site by installing dependencies and importing
 * configuration.
 *
 * @param array $recipe
 *   Fully loaded recipe implementation as results from recipes_load().
 *
 * @return none.
 */
function recipes_apply_form($form, $form_state, $recipe) {
  // Check the token before performing any action directly.
  if (!isset($_GET['token']) || !backdrop_valid_token($_GET['token'], current_path())) {
    backdrop_set_message(t('Invalid token'));
    backdrop_goto('admin/structure/recipes');
  }
  $form['#attached']['library'][] = array('recipes', 'recipes.admin');

  recipes_apply_recipe($recipe['signature']);

  // Go back to listing.
  backdrop_goto('admin/structure/recipes');
}

/**
 * Form: Force applies the config included in a recipe back to the state
 * provided by the recipe while applying the recipe.
 *
 * @param array $recipe
 *   Fully loaded recipe implementation as results from recipes_load().
 *
 * @return none.
 */
function recipes_force_apply_form($form, $form_state, $recipe) {
  $form['recipe'] = array(
    '#type' => 'value',
    '#value' => $recipe,
  );
  $markup = _recipes_do('list', 'config', $recipe['signature']);
  $markup .= '<p><strong>' . t('These active configuration files will be overwritten.') . '</strong></p>';
  $form['configuration'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  );

  // Confirm form.
  $replacements = array('%recipe' => $recipe['label'] ?? $recipe['name']);
  $message = t('Are you sure you want to force apply the <em>%recipe</em>?', $replacements);
  $path = 'admin/structure/recipes';
  $description = t('This action will force overwrite the configuration files listed above with the recipe\'s default state. This cannot be undone.');
  $yes_text = t('Force overwrite config files');
  $no_text = t('Cancel');

  return confirm_form($form, $message, $path, $description, $yes_text, $no_text, 'recipes_force_apply_confirm');
}

/**
 * Submit handler for force apply confirm form.
 * @see recipes_force_apply_form()
 */
function recipes_force_apply_form_submit(&$form, &$form_state) {
  $recipe = $form_state['values']['recipe'];

  // Force apply the recipe.
  recipes_apply_recipe($recipe['signature'], TRUE);

  // Set a message.
  $replacements = array('%recipe' => $recipe['label'] ?? $recipe['name']);
  $message = 'Configuration for <em>%recipe</em> force applied to recipe\'s original state.';
  backdrop_set_message(t($message, $replacements));

  // Go back to listing.
  backdrop_goto('admin/structure/recipes');
}

/**
 * Form: Detaches the recipe without changes by marking the recipe as inactive.
 *
 * @param array $recipe
 *   Fully loaded recipe implementation as results from recipes_load().
 *
 * @return none.
 */
function recipes_detach_form($form, $form_state, $recipe) {
  $form['recipe'] = array(
    '#type' => 'value',
    '#value' => $recipe,
  );
  $markup = '<p><strong>' . t('Detaching this recipe will mark it as inactive without any additional changes.') . '</strong></p>';
  $form['configuration'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  );

  // Confirm form.
  $replacements = array('%recipe' => $recipe['label'] ?? $recipe['name']);
  $message = t('Are you sure you want to detach the <em>%recipe</em> recipe?', $replacements);
  $path = 'admin/structure/recipes';
  $description = t('The recipe will be marked as inactive. Any configuration, state, or permissions changes the recipe made will remain. Any modules enabled by the recipe will not be disabled.');
  $yes_text = t('Detach recipe');
  $no_text = t('Cancel');

  return confirm_form($form, $message, $path, $description, $yes_text, $no_text, 'recipes_detach_confirm');
}

/**
 * Submit handler for detach confirm form.
 * @see recipes_detach_form()
 */
function recipes_detach_form_submit(&$form, &$form_state) {
  $recipe = $form_state['values']['recipe'];

  // Detach the recipe.
  recipes_mark_inactive($recipe['signature']);

  // Set a message.
  $replacements = array('%recipe' => $recipe['label'] ?? $recipe['name']);
  $message = 'The <em>%recipe</em> has been detached.';
  backdrop_set_message(t($message, $replacements));

  // Go back to listing.
  backdrop_goto('admin/structure/recipes');
}

/**
 * Form: Allows a recipe to be un-applied.
 *
 * @param array $recipe
 *   Fully loaded recipe implementation as results from recipes_load().
 *
 * @return confirm form.
 */
function recipes_rollback_form($form, $form_state, $recipe) {
  // Check the token before performing any action directly.
  if (!isset($_GET['token']) || !backdrop_valid_token($_GET['token'], current_path())) {
    backdrop_set_message(t('Invalid token'));
    backdrop_goto('admin/structure/recipes');
  }
  if (empty($recipe)) {
    backdrop_set_message(t('Recipe not found.'), 'error');
    backdrop_goto('admin/structure/recipes');
  }

  module_load_include('inc', 'recipes', 'recipes.admin');
  $rollback_signature = 'rollback|' . $recipe['signature'];
  $rollback_recipe = recipes_load($rollback_signature);
  if (empty($rollback_recipe)) {
    backdrop_set_message(t('Rollback recipe not found.'), 'error');
    backdrop_goto('admin/structure/recipes');
  }
  $form['recipe'] = array(
    '#type' => 'value',
    '#value' => $recipe,
  );

  $inspect_title = t('Rollback Recipe: @recipe', array(
    '@recipe' => $recipe['label'] ?? $recipe['name'],
  ));
  $inspect_subtitle = '<p><em>' . t('Rollback recipe changes') . '</em></p>';

  $inspect_actions = recipes_admin_operations($recipe);

  if (!empty($rollback_recipe['config'])) {
    $block_content = _recipes_do('list', 'config', $rollback_signature);
    $message = t('Configuration will not currently be rolled back due to potential conflicts with database setup, but you can manually remove fields, etc, through the UI.');
  }
  else {
    $block_content = '<p>' . t('No configuration found in this recipe.') . '</p>';
    $message = '';
  }
  $inspect_blocks[] = array(
    'title' => t('Bundled Configuration'),
    'message' => $message ?? '',
    'content' => $block_content,
  );

  if (!empty($rollback_recipe['dependencies'])) {
    $block_content = _recipes_do('list', 'dependencies', $rollback_signature);
    $message = t('The dependencies will be rolled back to the last saved state, as shown below.');
  }
  else {
    $block_content = '<p>' . t('No dependencies found in this recipe.') . '</p>';
    $message = '';
  }
  $inspect_blocks[] = array(
    'title' => t('Dependencies'),
    'message' => $message ?? '',
    'content' => $block_content,
  );

  if (!empty($rollback_recipe['overrides'])) {
    $block_content = _recipes_do('list', 'overrides', $rollback_signature);
    $message = t('The overrides will be rolled back to the last saved state, as shown below.');
  }
  else {
    $block_content = '<p>' . t('No overrides found in this recipe.') . '</p>';
    $message = '';
  }
  $inspect_blocks[] = array('title' => t('Overrides'),
    'message' => $message ?? '',
    'content' => $block_content,
  );

  $form['configuration'] = array(
    'type' => 'markup',
    '#markup' => theme('recipes_inspect', array(
      'title' => $inspect_title,
      'subtitle' => $inspect_subtitle,
      'actions' => (!empty($inspect_actions['#links']) ? $inspect_actions : array()),
      'blocks' => $inspect_blocks,
    )),
  );

  // Confirm form.
  $replacements = array('%recipe' => $recipe['label'] ?? $recipe['name']);
  $message = t('Are you sure you want to roll back the <em>%recipe</em> recipe?', $replacements);
  $path = 'admin/structure/recipes';
  $description = '<p>' . t('Depending on what else has changed since you applied the recipe, this rollback may not behave as expected. A full backup is recommended. Continue to attempt any rollbacks noted and then fully detach the recipe.') . '</p>';
  $yes_text = t('Rollback recipe');
  $no_text = t('Cancel');

  return confirm_form($form, $message, $path, $description, $yes_text, $no_text, 'recipes_rollback');
}

/**
 * Submit handler for confirm form.
 * @see recipes_rollback_form()
 */
function recipes_rollback_form_submit(&$form, $form_state) {
  $recipe = $form_state['values']['recipe'];
  $result = recipes_rollback_recipe($recipe['signature']);

  $message = 'The <em>%recipe</em> recipe has been rolled back.';
  backdrop_set_message(t($message, array('%recipe' => $recipe['label'] ?? $recipe['name'])));

  // Go back to listing.
  backdrop_goto('admin/structure/recipes');
}

/**
 * Form: Shows the contents of a recipe.
 *
 * @param $recipe
 *   Fully loaded recipe project as results from recipes_load(). Keys are:
 *   - label: human-readable name of the module.
 *   - machine_name: machine name of the recipe module project.
 *
 * @return string
 *   Rendered HTML for the inspection page.
 */
function recipes_inspect_form($form, $form_state, $recipe) {
  module_load_include('inc', 'recipes', 'recipes.admin');
  $markup = '';
  $form['recipe'] = array(
    '#type' => 'value',
    '#value' => $recipe,
  );

  $inspect_title = t('Recipe: @recipe', array(
    '@recipe' => $recipe['label'] ?? $recipe['name'],
  ));
  $inspect_subtitle = '<p><em>' . $recipe['signature'] . '</em></p>';

  $inspect_actions = recipes_admin_operations($recipe);
  unset($inspect_actions['#links']['inspect']);

  $block_content = _recipes_do('list', 'config', $recipe['signature']);
  $inspect_blocks[] = array(
    'title' => t('Bundled Configuration'),
    'message' => '',
    'content' => $block_content,
  );
  if (!empty($recipe['dependencies'])) {
    $block_content = _recipes_do('list', 'dependencies', $recipe['signature']);
  }
  else {
    $block_content = '<p>' . t('No dependencies found in this recipe.') . '</p>';
  }
  $inspect_blocks[] = array(
    'title' => t('Dependencies'),
    'message' => '',
    'content' => $block_content,
  );
  $block_content = _recipes_do('list', 'overrides', $recipe['signature']);
  $inspect_blocks[] = array(
    'title' => t('Overrides'),
    'message' => '',
    'content' => $block_content,
  );

  $element = array();
  $element['return'] = array(
    '#type' => 'dropbutton',
    '#links' => array(
      'edit-details' => array(
        'title' => t('Back to recipe list'),
        'href' => 'admin/structure/recipes',
      ),
    ),
  );

  $markup .= theme('recipes_inspect', array(
    'page' => $element,
    'title' => $inspect_title,
    'actions' => $inspect_actions,
    'subtitle' => $inspect_subtitle,
    'blocks' => $inspect_blocks ?? array(),
  ));

  $form['configuration'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  );

  return $form;
}

/**
 * Revert config by importing over existing files.
 *
 * @see config_install_default_config()
 */
function recipes_overwrite_config($project) {
  $project_path = backdrop_get_path('module', $project);
  $project_config_dir = $project_path . '/config';
  if (is_dir($project_config_dir)) {
    $storage = new ConfigFileStorage($project_config_dir);
    $files = glob($project_config_dir . '/*.json');
    foreach ($files as $file) {
      // Load config data into the active store and write it out to the
      // file system in the Backdrop config directory. Note the config name
      // needs to be the same as the file name WITHOUT the extension.
      $parts = explode('/', $file);
      $file = array_pop($parts);
      $file_config_name = str_replace('.json', '', $file);
      $data = $storage->read($file_config_name);
      $config = config($file_config_name);

      // We overwrite existing configuration here..
      $config->setData($data);
      module_invoke_all('config_create', $config);
      $config->save();
    }
  }
}
