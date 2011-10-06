<?php
/**
 * @file
 * Written by Henri MEDOT <henri.medot[AT]absyx[DOT]fr>
 * http://www.absyx.fr
 */

/**
 * Implementation of hook_ckeditor_link_TYPE_autocomplete().
 */
function ckeditor_link_ckeditor_link_menu_autocomplete($string) {
  $matches = array();

  $menus = array_keys(array_filter(variable_get('ckeditor_link_autocomplete_menus', array())));
  if (count($menus)) {
    $query = db_select('menu_links', 'ml')
      ->fields('ml', array('link_path', 'link_title'))
      ->condition('ml.link_title', '%'. $string .'%', 'LIKE')
      ->condition('ml.hidden', 0)
      ->condition('ml.external', 0)
      ->orderBy('ml.link_title')
      ->range(0, 10);
    if (!in_array('- any -', $menus)) {
      $query->condition('ml.menu_name', $menus, 'IN');
    }
    $result = $query->execute();
    foreach ($result as $item) {
      $matches[$item->link_path] = $item->link_title;
    }
  }

  return $matches;
}

/**
 * Implementation of hook_ckeditor_link_TYPE_settings().
 */
function ckeditor_link_ckeditor_link_menu_settings() {
  $form['menu'] = array(
    '#type' => 'fieldset',
    '#title' => t('Menu items'),
  );
  $form['menu']['ckeditor_link_autocomplete_menus'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Menus'),
    '#options' => array('- any -' => t('<em>Any menu</em>')) + array_map('check_plain', menu_get_menus()),
    '#default_value' => variable_get('ckeditor_link_autocomplete_menus', array()),
    '#description' => t('Select the menus to be available as autocomplete suggestions.'),
  );

  return $form;
}
