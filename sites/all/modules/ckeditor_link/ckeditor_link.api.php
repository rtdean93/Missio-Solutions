<?php

/**
 * @file
 * Hooks that can be implemented by other modules to extend CKEditor Link.
 */

/**
 * Declare the path types handled by the module.
 *
 * @return
 *   An array of the types handled by the module.
 *   Each value is either a type name or a sub-array with the following
 *   attributes:
 *    - 'type'
 *         The type name. Required.
 *    - 'file'
 *         A file that will be included before other hooks are invoked.
 *         The file should be relative to the implementing module's directory.
 */
function hook_ckeditor_link_types() {
  return array(
    'mytype1',
    array('type' => 'mytype2', 'file' => 'includes/mymodule.mytype2.inc'),
  );
}

/**
 * Alter types.
 *
 * @param $types
 *   The types returned by hook_ckeditor_link_types(). The types are keyed by
 *   'MODULE.TYPE' for easy lookup.
 *
 * @see hook_ckeditor_link_types().
 */
function hook_ckeditor_link_types_alter(&$types) {
  // Change types
}

/**
 * Get autocomplete suggestions for the given string.
 *
 * Implementing modules should return only suggestions the current user has
 * access to.
 *
 * @param $string
 *   The string to autocomplete.
 *
 * @return
 *   An array of suggestions where keys are non-aliased internal paths
 *   and values are titles.
 */
function hook_ckeditor_link_TYPE_autocomplete($string) {
  $matches = array();

  $matches['the-path/123'] = 'The title 1';
  $matches['the-path/the-path-2/5'] = 'The title 2';

  return $matches;
}

/**
 * Convert an internal path into an aliased and, if applicable, language
 * prefixed URL.
 *
 * @param $path
 *    The internal path to convert.
 * @param $langcode
 *    The language prefix of the path if any, the language code of the text to
 *    be filtered otherwise. It should only be used as a fallback when the
 *    content being linked to does not have any intrisic language.
 *
 * @return
 *    An URL alias, or nothing if the implementing module is not responsible
 *    for the given path.
 */
function hook_ckeditor_link_TYPE_url($path, $langcode = '') {
  //
}

/**
 * Add settings to the CKEditor Link settings form.
 *
 * @return
 *   An array containing the form elements to add.
 */
function hook_ckeditor_link_TYPE_settings() {
  $form = array(
    //
  );
  return $form;
}
