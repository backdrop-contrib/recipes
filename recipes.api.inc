<?php
/**
 * @file
 * Provides an admin interface for managing recipes.
 */

/**
 * Declares information about recipes actions.
 *
 * A recipes action consists of two parts:
 * - A definition (returned by this hook)
 * - A callback function which performs the action.
 *   By convention these functions are named MODULE_FUNCTION_recipes_action.
 *
 * @return array
 *   An associative array of action descriptions. The keys of the array are the
 *   names of the action functions, and each corresponding value is an
 *   associative array with the following key-value pairs:
 *   - label: The human-readable name of the action, which should be passed
 *       through the t() function for translation.
 *   - callback: Optional. A function name that will execute the action. By
 *       default, a function with the name of the action will be used.
 *   - permissions: A list of permissions, any of which will enable access to
 *       this action. For example, in order to perform this action, a person
 *       must have either permission1 OR permission2.
 *   - requirements_callback: A callback function to check if requirements have
 *       been met to execute this action. Defaults to `recipes_requirements`
 *       which always returns TRUE.
 *   - file: Optional. Relative path to a file from the module's directory that
 *       contains the callback function.
 */
function hook_recipes_action_info() {
  $actions = array(
    'recipes_import' => array(
      'label' => t('Import'),
      'callback' => 'recipes_import_recipes_action',
      'permissions' => array('manage recipes'),
      'requirements_callback' => 'recipes_requirements_module_enabled',
      'file' => 'recipes.operations.inc',
    ),
    'recipes_inspect' => array(
      'label' => t('Inspect'),
      'callback' => 'recipes_inspect_recipes_action',
      'permissions' => array('manage recipes', 'inspect recipes'),
      'file' => 'recipes.operations.inc',
    ),
  );

  return $actions;
}
