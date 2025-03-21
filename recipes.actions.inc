<?php
/**
 * @file
 * Recipe actions.
 */

/**
 * Form: Applies a recipe to a site by installing as a module.
 *
 * @param $recipe
 *   Fully loaded recipe project as results from recipe_load(). Keys are:
 *   - label: human-readable name of the module.
 *   - machine_name: machine name of the recipe module project.
 *
 * @return none.
 */
function recipes_import_form($form, $form_state, $recipe) {
  // Check the token before performing any action directly.
  if (!isset($_GET['token']) || !backdrop_valid_token($_GET['token'], current_path())) {
    backdrop_set_message(t('Invalid token'));
    backdrop_goto('admin/structure/recipes');
  }

  // Enabling the recipe will import the config.
  module_enable(array($recipe['machine_name']), TRUE);

  // Do we want to disable it again too?  - make a setting for this.
  //module_disable(array($recipe['machine_name']));

  // Set a mesage.
  $message = 'The <em>%recipe</em> has been applied to this site.';
  $replacements = array('%recipe' => recipes_clean_name($recipe['label']));
  backdrop_set_message(t($message, $replacements));

  // Go back to listing.
  backdrop_goto('admin/structure/recipes');
  // No return value to return to recipes listing.
  return array();
}

/**
 * Form: Reverts the config included in a recipe back to the state provided by the recipe.
 *
 * @param $recipe
 *   Fully loaded recipe project as results from recipe_load(). Keys are:
 *   - label: human-readable name of the module.
 *   - machine_name: machine name of the recipe module project.
 *
 * @return confirm form.
 */
function recipes_revert_form($form, $form_state, $recipe) {
  $form['recipe'] = array(
    '#type' => 'value',
    '#value' => $recipe,
  );

  $markup = _recipes_list_config_files($recipe);
  $markup .= '<p><strong>' . t('These active configuration files will be overwritten.') . '</strong></p>';
  $form['configuration'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  );

  // Confirm form.
  $replacements = array('%recipe' => recipes_clean_name($recipe['label']));
  $message = t('Are you sure you want to revert the <em>%recipe</em> to the default state?', $replacements);
  $path = 'admin/structure/recipes';
  $description = NULL; // Cannot be undone.
  $yes_text = t('Overwrite config files');
  $no_text = t('Cancel');

  return confirm_form($form, $message, $path, $description, $yes_text, $no_text, 'recipes_revert_confirm');
}

/**
 * Submit handler for revert confirm form.
 * @see recipes_revert().
 */
function recipes_revert_form_submit(&$form, &$form_state) {
  $recipe = $form_state['values']['recipe'];

  // @todo Will this revert? needs testing.
  recipes_overwrite_config($recipe['machine_name']);

  // Set a message.
  $replacements = array('%recipe' => recipes_clean_name($recipe['label']));
  $message = 'Configuration for <em>%recipe</em> revered to original state.';
  backdrop_set_message(t($message, $replacements));

  // Go back to listing.
  backdrop_goto('admin/structure/recipes');
}

/**
 * Form: Extracts the config included in a recipe.
 *
 * @param $recipe
 *   Fully loaded recipe project as results from recipe_load(). Keys are:
 *   - label: human-readable name of the module.
 *   - machine_name: machine name of the recipe module project.
 *
 * @return confirm form.
 */
function recipes_export_form($form, $form_state, $recipe) {
  $form['recipe'] = array(
    '#type' => 'value',
    '#value' => $recipe,
  );

  $markup = _recipes_list_config_files($recipe);
  $back_link = l(t('Back to recipe list'), 'admin/structure/recipes');
  $markup .= '<p>' . $back_link . '</p>';

  $form['configuration'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  );

  // Confirm form.
  $replacements = array('%recipe' => recipes_clean_name($recipe['label']));
  $message = 'Are you sure you want to export the configuration files for the <em>%recipe</em>?';
  $question = t($message, $replacements);
  $path = 'admin/structure/recipes';
  $description = NULL; // Cannot be undone.
  $yes_text = t('Extract recipe');
  $no_text = t('Cancel');
  $name = 'recipes_export_confirm';

  return confirm_form($form, $question, $path, $description, $yes_text, $no_text, 'recipes_export');
}

/**
 * Submit handler for confirm form.
 * @see recipes_export().
 */
function recipes_export_form_submit(&$form, $form_state) {
  $recipe = $form_state['values']['recipe'];

  // Export the configuration.
  // @todo

  // Set a message.
  $replacements = array('%recipe' => recipes_clean_name($recipe['label']));
  $message = 'Configuration for <em>%recipe</em> has been revered to the original state.';
  backdrop_set_message(t($message, $replacements));

  // Go back to listing.
  backdrop_goto('admin/structure/recipes');
}

/**
 * Form: ALlows a recipe to be un-applied.
 *
 * @param $recipe
 *   Fully loaded recipe project as results from recipe_load(). Keys are:
 *   - label: human-readable name of the module.
 *   - machine_name: machine name of the recipe module project.
 *
 * @return confirm form.
 */
function recipes_remove_form($form, $form_state, $recipe) {
  $form['recipe'] = array(
    '#type' => 'value',
    '#value' => $recipe,
  );

  $markup = _recipes_list_config_files($recipe);
  // @todo switch summary
  //$markup .= '<p><strong>' . t('These active configuration files will be removed.') . '</strong></p>';
  $markup .= '<p>' . t('These config files will remain, but the recipe module will be uninstalled. ') . '</p>';
  $form['configuration'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  );

  // Confirm form.
  $replacements = array('%recipe' => recipes_clean_name($recipe['label']));
  $message = t('Are you sure you want to remove the configuration files for the <em>%recipe</em>?', $replacements);
  $path = 'admin/structure/recipes';
  $description = '';
  //$description = NULL; // Cannot be undone.
  //$yes_text = t('Remove config files'); // @todo Uncomment when soon happens
  $yes_text = t('Uninstall recipe');
  $no_text = t('Cancel');

  return confirm_form($form, $message, $path, $description, $yes_text, $no_text, 'recipes_remove');
}

/**
 * Submit handler for confirm form.
 * @see recipes_remove().
 */
function recipes_remove_form_submit(&$form, $form_state) {
  $recipe = $form_state['values']['recipe'];

  module_disable(array($recipe['machine_name']));
  backdrop_uninstall_modules(array($recipe['machine_name']));
  // @todo Figure out how to delete only the config added by this recipe.


  $replacements = array('%recipe' => recipes_clean_name($recipe['label']));
  // @todo replace message
  //$message = 'Configuration for <em>%recipe</em> removed from active.';
  $message = 'The <em>%recipe</em> module has been uninstalled.';
  backdrop_set_message(t($message, $replacements));

  // Go back to listing.
  backdrop_goto('admin/structure/recipes');
}

/**
 * Form: Shows the contents of a recipe.
 *
 * @param $recipe
 *   Fully loaded recipe project as results from recipe_load(). Keys are:
 *   - label: human-readable name of the module.
 *   - machine_name: machine name of the recipe module project.
 *
 * @return string
 *   Rendered HTML for the inspection page.
 */
function recipes_inspect_form($form, $form_state, $recipe) {
  $form['recipe'] = array(
    '#type' => 'value',
    '#value' => $recipe,
  );

  $markup = _recipes_list_config_files($recipe);

  $element['return'] = array(
    '#type' => 'dropbutton',
    '#links' => array(
      'edit-details' => array(
        'title' => t('Back to recipe list'),
        'href' => 'admin/structure/recipes',
      ),
    ),
  );

  $markup .= backdrop_render($element);

  $form['configuration'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  );

  return $form;
}

/**
 * Generates a list of config files contained in a recipe.
 */
function _recipes_list_config_files($recipe) {
  $replacements = array('%recipe' => recipes_clean_name($recipe['label']));
  $message = t('The <em>%recipe</em> recipe contains the following configuration files:', $replacements);
  $markup = '<p>' . $message . '</p>';

  $config_path = backdrop_get_path('module', $recipe['machine_name']) . '/config';
  $files = array_diff(scandir($config_path), array('..', '.'));

  $markup .= '<pre>';
  $markup .= theme('item_list', array('items' => $files));
  $markup .= '</pre>';

  return $markup;
}

/**
 * Cleans the name of the recipe module and removes the word 'recipe';
 */
function recipes_clean_name($name) {
  $lower_name = strtolower(check_plain($name));
  $clean_name = trim(str_replace('recipe', '', $lower_name));
  return ucfirst($clean_name) . ' ' . t('Recipe');
}

/**
 * Revert config by importing over existing files.
 *
 * @see config_install_default_config().
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
