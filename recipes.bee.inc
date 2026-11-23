<?php
/**
 * @file
 * Bee recipe integration.
 */

/**
 * Implements hook_bee_command().
 */
function recipes_bee_command() {
  $items = array();

  // List all available Recipes.
  $items['recipe:list'] = array(
    'description' => bt('Lists all available recipes.'),
    'aliases' => array('recipe-list'),
    'bootstrap' => BACKDROP_BOOTSTRAP_FULL,
    'group' => 'recipes',
    'arguments' => array(
      'type' => bt('Optional. The type of recipes to list, defaulting to all. Other values could be "module", or "theme".'),
    ),
    'optional_arguments' => array('type'),
    'options' => array(
      'verbose' => array(
        'description' => bt('Show verbose output.'),
        'short' => 'v',
      ),
    ),
    'callback' => 'bee_recipe_list_execute',
    'examples' => array(
      'bee recipe:list module' => bt('Lists all module recipes.'),
      'bee recipe:list theme' => bt('Lists all theme recipes.'),
      'bee recipe:list' => bt('Lists all recipes.'),
      'bee recipe:list all verbose' => bt('Lists all recipes with verbose output.'),
    ),
  );

  // Apply a specific Recipe.
  $items['recipe:apply'] = array(
    'description' => bt('Applies a specific recipe.'),
    'aliases' => array('recipe-apply'),
    'bootstrap' => BACKDROP_BOOTSTRAP_FULL,
    'group' => 'recipes',
    'arguments' => array(
      'recipe' => bt('Required. The signature of the recipe to apply.'),
    ),
    'options' => array(
      'force' => array(
        'description' => bt('Force apply the recipe, overwriting existing config if needed.'),
        'short' => 'f',
      ),
    ),
    'callback' => 'bee_recipe_apply_execute',
    'examples' => array(
      'bee recipe:apply module:recipes:test-recipe' => bt('Applies the recipe with signature "module:recipes:test-recipe".'),
      'bee recipe:apply module:recipes:test-recipe force' => bt('Applies the recipe with signature "module:recipes:test-recipe" with force option.'),
    ),
  );

  // Detach a specific Recipe.
  $items['recipe:detach'] = array(
    'description' => bt('Detaches a specific recipe.'),
    'aliases' => array('recipe-detach'),
    'bootstrap' => BACKDROP_BOOTSTRAP_FULL,
    'group' => 'recipes',
    'arguments' => array(
      'recipe' => bt('Required. The signature of the recipe to detach.'),
    ),
    'callback' => 'bee_recipe_detach_execute',
    'examples' => array(
      'bee recipe:detach module:recipes:test-recipe' => bt('Detaches the recipe with signature "module:recipes:test-recipe".'),
    ),
  );

  return $items;
}

/**
 * Bee callback: Executes the recipe:list command.
 */
function bee_recipe_list_execute($arguments, $options = array()) {
  echo "\n";
  $valid_types = array('all', 'module', 'theme');
  $type = isset($arguments['type']) ? $arguments['type'] : 'all';
  if (!in_array($type, $valid_types)) {
    $type = 'all';
    bee_render_text(array(
      'value' => bt('Invalid recipe type specified, showing all recipes.'),
    ));
  }
  else {
    bee_render_text(array(
      'value' => bt('Listing recipes of type: @type', array('@type' => $type)),
    ));
  }
  bee_render_text(array('value' => "-------------------------------"));
  $recipes = recipes_load_multiple();
  if (empty($recipes)) {
    bee_render_text(array(
      'value' => bt('No recipes found.'),
    ));
    return;
  }
  else {
    foreach ($recipes as $recipe) {
      if ($type == 'all' || $recipe['project-type'] == $type) {
        $status = recipes_get_status($recipe['signature']);
        $color = $status['key'] == RECIPES_STATUS_INSTALLED ? 'green' : 'blue';
        bee_render_text(array(
          'value' => " • " . $recipe['label'] . ': ' . bee_format_text($recipe['signature'], $color) . ' (' . $status['key'] . ')',
        ));
        if (!empty($options['verbose']) && $options['verbose'] == 'verbose' && !empty($recipe['description'])) {
          bee_render_text(array(
            'value' => bt('    @description', array('@description' => $recipe['description'])),
          ));
          echo "\n";
        }
      }
    }
    echo "\n\n";
  }
}

/**
 * Bee callback: Executes the recipe:apply command.
 */
function bee_recipe_apply_execute($arguments, $options = array()) {
  echo "\n";
  if (empty($arguments['recipe'])) {
    bee_message(bt('No recipe specified.'), 'error');
    return;
  }
  $recipes = recipes_load_multiple();
  $recipe_signature = $arguments['recipe'];
  if (!isset($recipes[$recipe_signature])) {
    bee_message(bt('Recipe not found: @recipe', array('@recipe' => $recipe_signature)), 'error');
    return;
  }
  $force = !empty($options['force']);
  if ($force) {
    $continue = bee_confirm(bt('Are you sure you want to force apply the recipe: @recipe? This may overwrite existing configurations.', array('@recipe' => $recipe_signature)));
    if (!$continue) {
      return;
    }
  }

  $suffix = $force ? ' (with force)' : '';
  bee_render_text(array(
    'value' => bt('Applying recipe: @recipe@suffix', array('@recipe' => bee_format_text($recipe_signature, 'green'), '@suffix' => $suffix)),
  ));

  recipes_apply_recipe($recipe_signature, $force);

  echo "\n";
}

/**
 * Bee callback: Executes the recipe:detach command.
 */
function bee_recipe_detach_execute($arguments, $options = array()) {
  echo "\n";
  if (empty($arguments['recipe'])) {
    bee_message(bt('No recipe specified.'), 'error');
    return;
  }
  $recipe_signature = $arguments['recipe'];
  $recipe = recipes_load($recipe_signature);
  if (empty($recipe)) {
    bee_message(bt('Recipe not found: @recipe', array('@recipe' => bee_format_text($recipe_signature, 'blue'))), 'error');
    return;
  }
  if (!recipes_is_active($recipe_signature)) {
    bee_message(bt('Recipe is already detached: @recipe', array('@recipe' => bee_format_text($recipe_signature, 'blue'))), 'error');
    return;
  }

  $continue = bee_confirm(bt('Are you sure you want to detach the recipe: @recipe?', array('@recipe' => bee_format_text($recipe_signature, 'green'))));
  if (!$continue) {
    return;
  }

  bee_render_text(array(
    'value' => bt('Detaching recipe: @recipe', array('@recipe' => bee_format_text($recipe_signature, 'blue'))),
  ));

  recipes_mark_inactive($recipe_signature);

  echo "\n";
}
