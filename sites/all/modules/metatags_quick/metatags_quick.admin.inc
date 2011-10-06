<?php
function metatags_quick_admin_settings() {
  $current_settings = variable_get('metatags_quick_settings', _metatags_quick_settings_default());
  $module_path = drupal_get_path('module', 'metatags_quick');
  $fields = field_info_fields();
  $metatags_found = FALSE;

  include_once $module_path . '/known_tags.inc';
  $known_tags = _metatags_quick_known_fields();
  
  $form['standard_tags'] = array(
    '#type' => 'fieldset',
    '#title' => t('Create and attach'),
    '#description' => t('The following meta tags are known to the module. However, you are not limited to this list and can define tags of your own.'),
    '#collapsible' => TRUE,
    '#attached' => array(
      'js' => array($module_path . '/js/admin.js'),
      'css' => array($module_path . '/css/admin.css'),
    ),
  );
  $form['standard_tags']['hint'] = array(
    '#markup' => t('Hint: press on the bundle name to see individual entity types'),
  );
  
  $field_instances = field_info_instances();

  // Build the sortable table header.
  $header = array(
    'title' => array('data' => t('Bundle/entity')),
  );
  foreach ($known_tags as $name => $tag) {
    $header[$name] = $tag['title'];
  }
  //$header['_select_all'] = t('Select all');
  
  foreach (field_info_bundles() as $entity_type => $bundles) {
    $entity_info = entity_get_info($entity_type);
    if (!$entity_info['fieldable']) {
      continue;
    }
    $options[$entity_type]['data'] = array(
      'title' => array(
        'class' => array('entity_type_collapsible', 'entity_type_collapsed', "entity_name_$entity_type"),
        'data' => $entity_info['label']
      )
    );
    foreach ($known_tags as $name => $tag) {
      $bundle_workable[$name] = FALSE;
      $options[$entity_type]['data'][$name] = array(
        'data' => array(
          '#type' => 'checkbox',
          '#attributes' => array('class' => array('cb_bundle_parent', "cb_bundle_name_{$entity_type}_{$name}")),      
          '#return_value' => 1,
          '#checked' => FALSE,
        ),
      );
    }
    /*$options[$entity_type]['data']['_select_all'] = array(
      'data' => array(
      '#type' => 'checkbox',
      '#return_value' => 1,
      '#checked' => FALSE,
      ),
    );*/
    
    // How do we mark that specific meta is already attached to bundle
    $checked_markup = array(
      '#markup' => theme('image', 
        array(
          'path' => 'misc/watchdog-ok.png',
          'width' => 18,
          'height' => 18,
          'alt' => 'ok',
          'title' => 'ok',
        )),
    );
        
    foreach ($bundles as $key => $bundle) {
      // Which meta tags are set for this bundle?
      $meta_set = array();
      foreach ($field_instances[$entity_type][$key] as $bundle_instance) {
        if ($bundle_instance['widget']['module'] == 'metatags_quick') {
          $field_info = field_info_field_by_id($bundle_instance['field_id']);
          $meta_set[$field_info['settings']['meta_name']] = 1;
        }        
      }
      
      $options[$entity_type . '_' . $key] = array(
        'class' => array('entity_type_children', "entity_child_$entity_type"),
        'style' => 'display: none',
        'data' => array(
          'title' => array(
            'class' => array('entity_type_child_title'),
            'data' => $bundle['label'],
          ),
        ),
      );
      foreach ($known_tags as $name => $tag) {
        if (empty($meta_set[$name])) {
          // Mark parent bundle as workable - display checkbox.
          $bundle_workable[$name] = TRUE;
          $options[$entity_type . '_' . $key]['data'][$name] = array(
            'data' => array(
              '#name' => $entity_type . '[' . $key . '][' . $name . ']',
              '#type' => 'checkbox',
              '#attributes' => array('class' => array('cb_bundle_child', "cb_child_{$entity_type}_{$name}")),
              '#return_value' => 1,
              '#checked' => FALSE,
            )
          );
        }
        else {
          $options[$entity_type . '_' . $key]['data'][$name]['data'] = $checked_markup; 
        }
      }
      /*$options[$entity_type . '_' . $key]['data']['_select_all']['data'] = array(
        '#type' => 'checkbox',
        '#return_value' => 1,
        '#checked' => FALSE,
      );*/
    }
    // Now check if we have completely set bundles
    foreach ($known_tags as $name => $tag) {
      if (!$bundle_workable[$name]) {
        $options[$entity_type]['data'][$name]['data'] = $checked_markup; 
      }
    }
  }
  
  $form['standard_tags']['existing'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $options,
    '#empty' => t('No content available.'),
  );
  
  $form['standard_tags']['basic_init_op'] = array(
    '#type' => 'submit',
    '#value' => t('Attach'),
  );
  
  $form['front_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Front page meta tags'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  foreach ($fields as $key => $field) {
    if ($field['module'] != 'metatags_quick') {
      continue;
    }
    $metatags_found = TRUE;
    $form_key = $field['settings']['meta_name'];
    $field_form = array(
      '#title' => $key,
      '#type' => 'textarea',
      '#default_value' => empty($current_settings['front_values'][$form_key]) ? '' : $current_settings['front_values'][$form_key],
    );
    $form['front_settings'][$form_key] = $field_form;
  }
  if (!$metatags_found) {
    $form['front_settings']['basic_init'] = array(
      '#markup' => t('No meta tags found in your installation') . '<br/>',
    );
  }
  
  $form['front_settings']['use_front'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use front page meta tags'),
    '#description' => t('Don\'t use that option if your front page is a fieldable entity (i.e. single node, taxonomy term etc.)'),
    '#default_value' => $current_settings['use_front'],
    '#return_value' => 1,
  );
  
  
  $form['op'] = array(
    '#value' => t('Submit'),
    '#type' => 'submit', 
  );
  return $form;
}

function metatags_quick_admin_settings_submit($form, &$form_state) {
  $new_settings = array(
    'use_front' => !empty($form_state['values']['use_front']),
  );
  if ($form_state['clicked_button']['#value'] == t('Attach')) {
    _metatags_quick_admin_fields_create_attach($form_state['input']);
  }
  else {
    foreach (field_info_fields() as $field_name => $field) {
      if ($field['module'] != 'metatags_quick') {
        continue;
      }
      $form_key = $field['settings']['meta_name'];
      $new_settings['front_values'][$form_key] = $form_state['values'][$form_key];
    }
  }
  variable_set('metatags_quick_settings', $new_settings);
  drupal_set_message(t('Meta tags (quick) settings saved'), 'status');
}

/*
 * Path-based meta tags editing form.
 * @param string $path - path being edited.
 */
function metatags_quick_admin_path() {
  global $language;
  $args = func_get_args();
  $form = array();
  if (count($args) > 2) {
    $path = $args[2];
  }
  else {
    return $form;
  }
  
  $form['meta_tags'] = array(
    '#type' => 'fieldset',
    '#title' => t('Edit meta tags'),
    '#collapsible' => TRUE,
  );
  $fields = field_info_fields();
  $metatags_found = FALSE;
  $meta_fields = array();
  foreach ($fields as $key => $field) {
    if ($field['module'] != 'metatags_quick') {
      continue;
    }
    $metatags_found = TRUE;
    $form_key = $field['settings']['meta_name'];
    $meta_fields[$field['id']] = array('key' => $key) ;
  }
  
  if (!$metatags_found) {
    $form['meta_tags']['basic_init'] = array(
      '#markup' => t('No meta tags found in your installation'),
    );
    $form['meta_tags']['basic_init_op'] = array(
      '#type' => 'submit',
      '#value' => t('Create basic meta tags'),
    );
  }
  else {  
    $current_values = db_select('metatags_quick_path_based', 'pv')
      ->condition('lang', $language->language)
      ->condition('path', $path)
      ->fields('pv', array('fid', 'value'))
      ->execute();
    
    foreach ($current_values as $row) {
      $meta_fields[$row->fid]['value'] = $row->value;
    }
    
    foreach ($meta_fields as $key => $settings) {
      $form_key = $fields[$meta_fields[$key]['key']]['settings']['meta_name'];
      $form_key = $key;
      $field_form = array(
        '#title' => $meta_fields[$key]['key'],
        '#type' => 'textarea',
        '#default_value' => isset($meta_fields[$key]['value']) ? $meta_fields[$key]['value'] : '',
      );
      $form['meta_tags'][$form_key] = $field_form;
      $form['meta_tags']["{$form_key}_set"] = array(
        '#type' => 'hidden',
        '#value' => isset($meta_fields[$key]['value']) ? '1' : '0',
      );
    }
  }
  $form['op'] = array(
    '#value' => t('Submit'),
    '#type' => 'submit', 
  );  
  return $form;
}

function metatags_quick_admin_path_submit($form, &$form_state) {
  $path = $form_state['build_info']['args'][0];
  if (!$path) {
    form_set_error();
    drupal_set_message(t('Wrong path'), 'error');
    return;
  }
  
  // Now save data.
  $fields = field_info_fields();
  foreach ($fields as $key => $field) {
    if ($field['module'] != 'metatags_quick') {
      continue;
    }
    $form_key = $field['id'];
    if ($form_state['values']["{$form_key}_set"]) {
      db_update('metatags_quick_path_based')
        ->condition('lang', $GLOBALS['language']->language)
        ->condition('path', $path)
        ->condition('fid', $field['id'])
        ->fields(array('value' => $form_state['values'][$form_key]))
        ->execute();
    }
    else {
      db_insert('metatags_quick_path_based')->fields(array(
        'lang' => $GLOBALS['language']->language,
        'path' => $path,
        'fid' => $field['id'],
        'value' => $form_state['values'][$form_key],
      ))->execute();
    }
  }
  
  drupal_set_message('Meta tags updated', 'status');
}

// Create known meta fields.
function _metatags_quick_admin_fields_create_attach($input) {
  foreach (field_info_bundles() as $entity_type => $bundles) {
    $entity_info = entity_get_info($entity_type);
    if (!$entity_info['fieldable']) {
      continue;
    }
    
    foreach ($bundles as $key => $bundle) {
      if (isset($input[$entity_type][$key])) {
        foreach ($input[$entity_type][$key] as $meta_name => $meta_value) {
          _metatags_quick_field_attach($entity_type, $key, $meta_name);
        }
      }
    }
  }
}
  
function _metatags_quick_field_attach($entity_type, $bundle, $meta_name) {
  static $meta_fields = FALSE;
  static $field_id = array();
  static $known_tags = FALSE;
  
  // Get metatags_quick fields info
  if (!$meta_fields) {
    include_once drupal_get_path('module', 'metatags_quick') . '/known_tags.inc';
    $known_tags = _metatags_quick_known_fields();
    
    foreach(field_info_fields() as $name => $field_info) {
      if ($field_info['module'] == 'metatags_quick') {
        $meta_fields[$field_info['settings']['meta_name']] = $field_info;
        $field_id[$field_info['settings']['meta_name']] = $field_info['id'];
      }
    }
  }
  
  // Check if meta field exists, create if necessary.
  if (empty($field_id[$meta_name])) {
    // @todo: check if there is $known_tags[$meta_name]?
    // @todo: check for existing meta_* field (while is unlikely, but still possible).
    $field = array(
      'field_name' => "meta_$meta_name", 
      'type' => 'metatags_quick', 
      'module' => 'metatags_quick',
      'settings' => array('meta_name' => $meta_name), 
      'cardinality' => 1,
    );
    $field = field_create_field($field);
    $field_id[$meta_name] = $field['id'];
    $meta_fields[$meta_name] = $field;
  }
  else {
    $field = $meta_fields[$meta_name];
  }

  // Now create field instance attached to requested bundle
  $instance = array(
    'field_name' => $field['field_name'],
    'entity_type' => $entity_type,
    'bundle' => $bundle,
    'label' => $known_tags[$meta_name]['title'],
    'formatter' => 'metatags_quick_default',
    'widget' => array(
      'type' => $known_tags[$meta_name]['widget'],
      'weight' => 0,
    ),
  );
  field_create_instance($instance);
}
