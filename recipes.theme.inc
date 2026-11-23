<?php
/**
 * @file
 * Theme functions for recipes module.
 */

/**
 * Creates markup for a listing of recipes.
 */
function theme_recipes_listing($variables) {
  $form = $variables['form'];

  $rows = array();
  foreach (element_children($form) as $key) {
    $row = array();
    $row[] = $form[$key]['name'];
    $row[] = $form[$key]['description'];
    $row[] = $form[$key]['status'];
    $row[] = $form[$key]['operations'];
    $rows[] = $row;
  }

  // Render the form as a table.
  return theme('table', array(
    'header' => $form['#header'],
    'rows' => $rows,
    'empty' => t('No recipes available.'),
  ));
}

/**
 * Creates markup for an inspect form of a recipe.
 */
function theme_recipes_inspect($variables) {
  $inspect = array();
  backdrop_set_title($variables['title']);
  $inspect['subtitle'] = $variables['subtitle'] ?? '';
  $inspect['actions'] = $variables['actions'] ?? array();
  foreach ($variables['blocks'] as $key => $block) {
    if (!empty($block['message'])) {
      $block['content'] = '<p>' . $block['message'] . '</p>' . $block['content'];
    }
    $inspect['blocks'][] = array(
      '#theme' => 'admin_block',
      '#block' => array(
        'title' => $block['title'],
        'content' => $block['content'],
        'show' => TRUE,
      ),
    );
  }
  $inspect['back-link'] = $variables['back-link'] ?? array();
  return backdrop_render($inspect);
}

/**
 * Creates markup for recipes admin fieldsets (e.g. on the recipes listing page).
 *
 * @param array $variables
 *   Key 'form' is necessary for rendering.
 */
function theme_recipes_fieldset($variables) {
  $form = $variables['form'];

  $header = array(
    '',
    t('Name'),
    array('data' => t('Description'), 'class' => array(
      RESPONSIVE_PRIORITY_LOW,
    )),
    array('data' => t('Status'), 'class' => array(
      RESPONSIVE_PRIORITY_LOW,
    )),
    array('data' => t('Operations')),
  );

  // Iterate through all the recipes, which are children of this fieldset.
  $rows = array();
  foreach (element_children($form) as $key) {
    $status_key = $form[$key]['#status-key'] ?? 'unknown';
    $row_class = match ($status_key) {
      RECIPES_STATUS_AVAILABLE => 'report-info',
      RECIPES_STATUS_UNAVAILABLE => 'report-error',
      RECIPES_STATUS_CHANGED => 'report-warning',
      RECIPES_STATUS_INSTALLED => 'report-status',
      default => 'report-info',
    };
    $status_class = $status_key ? 'recipe--' . $status_key : 'recipe';
    $row = array(
      'row-icon' => array(
        'data' => '<div title="' . $status_key . '"><span class="element-invisible">' . $status_key . '</span></div>',
        'class' => 'report-icon',
      ),
    );
    $row[] = array('data' => $form[$key]['#name'] ?? 'name-unknown', 'class' => ['table-filter-text-source']);
    $row[] = array('data' => $form[$key]['#description'] ?? t('No description available.'));
    $row[] = array('data' => $form[$key]['#status'], 'class' => [$status_class]);
    $row[] = array('data' => $form[$key]['operations']);
    $rows[] = array(
      'data' => $row,
      'class' => array($row_class),
    );
    unset($form[$key]);
  }

  $table = array(
    'header' => $header,
    'rows' => $rows,
    'empty' => t('No recipes available.'),
  );

  return theme('table', $table);
}
