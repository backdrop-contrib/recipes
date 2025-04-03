<?php
/**
 * @file
 * Theme functions for recipes module.
 */

/**
 * Creates markup for the fieldset for recipes admin.
 *
 * @param array $variables
 *   Key 'form' is necessary for rendering.
 */
function theme_recipes_fieldset($variables) {
  $form = $variables['form'];

  // Individual table headers.
  $rows = array();

  // Iterate through all the recipes, which are children of this fieldset.
  foreach (element_children($form) as $key) {
    // Stick the key into $module for easier access.
    $module = $form[$key];
    // Create the row for the table.
    $row = array();
    $module['#requires'] = array_filter($module['#requires']);
    $module['#required_by'] = array_filter($module['#required_by']);

    $requires = !empty($module['#requires']);
    $required_by = !empty($module['#required_by']);
    $requirements = '';
    $toggle = '';

    // Add the first column: module label and expand/collapse functionality.
    $row[] = array('class' => array('module'), 'data' => '<label id="module-' . $key . '" class="module-name table-filter-text-source">' . backdrop_render($module['name']) . '</label>');

    // Add the second column: module version number.
    $row[] = array('class' => array('version'), 'data' => backdrop_render($module['version']));

    // Add the description, along with any modules it requires.
    $details = '<div class="text table-filter-text-source"> ' . backdrop_render($module['description']) . '</div>';

    // Add the third column: description, along with any modules it requires.
    $row[] = array('class' => array('description'), 'data' => $details);

    $requirements = '';
    if ($requires) {
      $requirements .= '<div class="admin-requirements">' . t('Requires:') . ' ' . implode(', ', $module['#requires']) . '</div>';
    }
    if ($required_by) {
      $requirements .= '<div class="admin-requirements">' . t('Required by:') . ' ' . implode(', ', $module['#required_by']) . '</div>';
    }

    // Add the fourth column: description, along with any modules it requires.
    $row[] = array('class' => array('requirements'), 'data' => $requirements);

    // Add the fifth column: links (such as configure or permissions) in their own columns.
    $row[] = array('data' => backdrop_render($module['links']), 'class' => array('operations'));

    $rows[] = $row;
  }

  $table = array(
    'header' => $form['#header'],
    'rows' => $rows,
    'empty' => t('No recipes available.'),
  );

  return theme('table', $table);
}
