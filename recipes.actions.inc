<?php
/**
 * @file
 * Recipe actions.
 */

/**
 * Menu callback: Applies a recipe to a site by installing as a module.
 *
 * @param $module
 *   Machine name of the recipe module.
 */
function recipes_import($info) {
  $module_list = array($info['project']);

  // Enabling the recipe will import the config.
  module_enable($module_list);
  //module_disable($module_list);

  $replacements = array('%recipe' => trim(str_replace('Recipe', '', $info['name'])));
  backdrop_set_message(t('The %recipe Recipe has been applied to this site.', $replacements));

  // No return value to return to recipes listing.
}

/**
 * Menu callback: Reverts the config included in a recipe back to the state provided by the recipe.
 *
 * @param $module
 *   Machine name of the recipe module.
 */
function recipes_revert($info) {
  $module = $info['project'];
  $name = $info['name'];

  $form['recipe'] = array(
    '#type' => 'value',
    '#value' => $module,
  );

  $markup = _recipes_list_config_files($info);
  $form['configuration'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  );

  $replacements = array('%recipe' => trim(str_replace('Recipe', '', $name)));
  $message = t("Are you sure you want to revert the %recipe recipe to it's default state?", $replacements);

  return confirm_form($form, $message, 'admin/structure/recipes', '', t('Revert recipe'), t('Cancel'));
}

/**
 * Submit handler for revert confirm form.
 */
function recipes_revert_submit($module) {
  recipes_import($module);
  backdrop_set_message(t('Recipe configuration revered to original state.'));
  // No return value to return to recipes listing.
}

/**
 * Menu callback: extracts the config included in a recipe.
 *
 * @param $module
 *   Machine name of the recipe module.
 */
function recipes_export($info) {
  $module = $info['project'];
  $name = $info['name'];

  $form['recipe'] = array(
    '#type' => 'value',
    '#value' => $module,
  );
  $markup = _recipes_list_config_files($info);
  $form['configuration'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  );

  $replacements = array('%recipe' => $name);
  $message = t('Are you sure you want to export the configuration files for %recipe?', $replacements);

  return confirm_form($form, $message, 'admin/structure/recipes', '', t('Extract recipe'), t('Cancel'));
}

/**
 * Submit handler for revert confirm form.
 */
function recipes_export_submit($module) {
  recipes_import($module);
  $replacements = array('%recipe' => $module);
  backdrop_set_message(t('Recipe configuration for $recipe revered to original state.', $replacements));
  // No return value to return to recipes listing.
}

/**
 * Menu callback: ALlows a recipe to be un-applied.
 *
 * @param $module
 *   Machine name of the recipe module.
 */
function recipes_remove($info) {
  $module = $info['project'];
  $name = $info['name'];

  $form['recipe'] = array(
    '#type' => 'value',
    '#value' => $module,
  );

  $markup = _recipes_list_config_files($info);
  $form['configuration'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  );

  $replacements = array('%recipe' => $name);
  $message = t('Are you sure you want to remove the configuration files for %recipe?', $replacements);

  return confirm_form($form, $message, 'admin/structure/recipes', '', t('Remove recipe'), t('Cancel'));
}

function recipes_remove_submit($info) {
  $module = $info['project'];
  $name = $info['name'];

  $replacements = array('%recipe' => $name);
  backdrop_set_message(t('Recipe configuration for %recipe revered to original state.'));
  // No return value to return to recipes listing.
}

/**
 * Menu callback: Shows the contents of a recipe.
 *
 * @param $module
 *   Machine name of the recipe module.
 */
function recipes_inspect($info) {
  $module = $info['project'];
  $name = $info['name'];

  $markup = _recipes_list_config_files($info);

  return $markup;
}

/**
 * Helper function: generate a list of config files contained in a recipe.
 */
function _recipes_list_config_files($info) {
  $module = $info['project'];
  $name = $info['name'];

  $replacements = array('%recipe' => $name);
  $markup = t('The %recipe recipe contains the following configuration files:', $replacements);

  $items = array('todo');
  $markup .= theme('item_list', array('items' => $items));

  return $markup;
}
