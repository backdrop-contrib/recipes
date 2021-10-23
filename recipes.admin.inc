<?php
/**
 * @file
 * Provides an admin interface for managing recipes.
 */

/**
 * Implements hook_menu().
 */
function recipes_admin($form, &$form_state) {
  $form = array('#config' => 'recipes.settings');

  $form['help'] = array(
    '#type' => 'help',
    '#markup' => t('The following modules are packages of configuration.'),
  );

  $cache = cache_get('recipes:recipes_list');
  if ($cache) {
    $projects = $cache->data;
  }
  else {
    $projects = system_rebuild_module_data();
  }

  $rows = array();
  foreach ($projects as $machine => $module) {
    if (isset($module->info['package']) && $module->info['package'] == 'recipe') {
      $row = array();
      $name = theme('label_machine_name', array(
        'label' => $module->info['name'],
        'machine_name' => $machine,
      ));
      $row[] = array('data' => '[ ]'); // @todo
      $row[] = array('data' => $name);
      $row[] = array('data' => $module->info['version']);
      $row[] = array('data' => $module->info['description']);
      $row[] = array('data' => $module->info['dependencies']);
      $row[] = array('data' => '@todo!');
      $rows[] = $row;
    }
  }

  if (empty($rows)) {
    $empty_text = t('No recipes found. Check that <code>package = recipe</code> exists in the .info file.');
    $rows[] = array(
      array('data' => $empty_text, 'colspan' => 2),
    );
  }

  $header = array(
    array('data' => '<span class="element-invisible">' . t('Enabled') . '</span>'),
    array('data' => t('Recipe Name')),
    array('data' => t('Version')),
    array('data' => t('Description')),
    array('data' => t('Dependencies')),
    array('data' => t('Operations')),
  );

  $form['table'] = array(
    '#type' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#attributes' => array(
      'class' => array('recipe-admin-table'),
    ),
  );

  $table = theme('table', array(
    'header' => $header,
    'rows' => $rows,
  ));

  $form['table'] = array(
    '#type' => 'markup',
    '#markup' => $table,
  );

/*
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Enable Recipe'),
  );
*/

  return $form;
}

/**
 * Submit handler for admin listing form.
 */
function recipes_admin_submit(&$form, &$form_state) {
  backdrop_set_message('@todo!');
}
