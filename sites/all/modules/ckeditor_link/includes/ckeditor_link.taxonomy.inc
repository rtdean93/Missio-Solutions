<?php
/**
 * @file
 * Written by Henri MEDOT <henri.medot[AT]absyx[DOT]fr>
 * http://www.absyx.fr
 */

/**
 * Implementation of hook_ckeditor_link_TYPE_autocomplete().
 */
function ckeditor_link_ckeditor_link_taxonomy_autocomplete($string) {
  $matches = array();

  $vocabularies = array_keys(array_filter(variable_get('ckeditor_link_autocomplete_vocabularies', array())));
  if (count($vocabularies)) {
    $query = db_select('taxonomy_term_data', 't')
      ->fields('t', array('tid', 'name'))
      ->condition('t.name', '%'. $string .'%', 'LIKE')
      ->orderBy('t.name')
      ->range(0, 10)
      ->addTag('term_access');
    if (!in_array('- any -', $vocabularies)) {
      $query->condition('t.vid', $vocabularies, 'IN');
    }
    $result = $query->execute();
    foreach ($result as $term) {
      $matches['taxonomy/term/'. $term->tid] = $term->name;
    }
  }

  return $matches;
}

/**
 * Implementation of hook_ckeditor_link_TYPE_url().
 */
function ckeditor_link_ckeditor_link_taxonomy_url($path) {
  if (!preg_match('`^taxonomy/term/(\d+)$`', $path, $matches)) {
    return;
  }

  $tid = $matches[1];
  $options = array();

  //TODO: Handle i18n.

  return url("taxonomy/term/$tid", $options);
}

/**
 * Implementation of hook_ckeditor_link_TYPE_settings().
 */
function ckeditor_link_ckeditor_link_taxonomy_settings() {
  $form['taxonomy'] = array(
    '#type' => 'fieldset',
    '#title' => t('Taxonomy terms'),
  );

  $vocabularies = taxonomy_get_vocabularies();
  $options = array('- any -' => t('<em>Any vocabulary</em>'));
  foreach ($vocabularies as $vid => $vocabulary) {
    $options[$vid] = check_plain($vocabulary->name);
  }
  $form['taxonomy']['ckeditor_link_autocomplete_vocabularies'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Vocabularies'),
    '#options' => $options,
    '#default_value' => variable_get('ckeditor_link_autocomplete_vocabularies', array()),
    '#description' => t('Select the vocabularies to be available as autocomplete suggestions.'),
  );

  return $form;
}
