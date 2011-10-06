<?php
/**
 * @file
 * Handles theme related functions such as html menus, dropdowns and links
 */

// PATHS

/**
 * UI Block Context Paths Menu
 *
 * $paths           : array of main_menu and block paths
 */
function ui_paths_menu($paths) {

  // Sellect All link if URL is 'root'
  $all_selected = FALSE;

  if (block_context_current_path() == '') {

    $menu = '<li><a class="selected" href="' . base_path() . 'admin/structure/block/">All</a></li>';
    $all_selected = TRUE;
  }
  else {
    $menu = '<li><a href="' . base_path() . 'admin/structure/block/">All</a></li>';
  }

  // Path links
  foreach ($paths as $path => $title) {

    // select link if path is current
    if (block_context_current_path() == $path || block_context_current_path() == html_entity_decode($path)) {
      $menu = $menu . '<li><a class="selected" href="' . base_path() . 'admin/structure/block/path/' . $path . '">' . $title . '</a></li>';
    }
    // deselect link if path is not current
    else {
      $menu = $menu . '<li><a href="' . base_path() . 'admin/structure/block/path/' . $path . '">' . $title . '</a></li>';
    }
  }

  return $menu;
}

/**
 * UI Selected Paths Dropdown Menu
 *
 * $all_paths       : array of main_menu paths/titles and all block paths
 * $block_key       : in the form 'module_delta'
 * $block_paths     : array of paths that this block is enabled for
 *
 * returns          : html dropdown menu for the block
 */
function ui_paths_dropdown($all_paths, $block_key, $block_paths) {

  // create default menu item
  $menu = '<option>All</option>';

  // only create dropdown menu items for blocks with paths
  if ($block_paths[0] != '') {

    // for each path item in the $all_paths array
    foreach ($all_paths as $path => $title) {

      // match the current $all_paths path with the last path of the block
      if ($path == htmlspecialchars(end($block_paths))) {

        $menu = $menu . '<option selected="selected" value="' . $path . '">' . $path . '</option>';
      }
      // or create normal dropdown item
      else {
        $menu = $menu . '<option value="' . $path . '">' . $path . '</option>';
      }
    }
  }
  // create dropdown menu for block with no paths
  else {
    foreach ($all_paths as $path => $title) {
      $menu = $menu . '<option value="' . $path . '">' . $path . '</option>';
    }
  }
  // wrap menu items inside select menu
  $menu ='<select name="blocks['  .  $block_key  .  '][path]">'.  $menu  .  '</select>' . ui_paths_list($block_paths) . '';
  return $menu;
}

/**
 * UI Unselected Paths List
 *
 * $paths           : array of this blocks paths
 * returns          : html list of paths in this block except the first (selected)
 */
function ui_paths_list($paths) {

  // if array more than one element (we remove first element anyway)
  if ( count($paths) > 1 ) {

    // remove first element from array
    array_pop($paths);

    $html = '';

    foreach ($paths as $key => $path) {

      $html = $html . '<li>' . htmlspecialchars($path) . '</li>'; // also converting internal "$lt;" to "<"
    }
    return '<span class="rest-links"><ul><li><em>As well as:</em></li>' . $html . '</ul></span>';
  }
  else return NULL;
}


// ROLES

/**
 * UI Block Context Roles Menu
 *
 * $roles   : array of roles in the form of key => role (1 => anonymous)
 */
function ui_roles_menu($roles) {

  // Sellect All link if URL is 'root'
  $all_selected = FALSE;

  // All
  if (block_context_current_rid() == '') {

    $menu = '<li><a class="selected" href="' . base_path() . 'admin/structure/block/role">All</a></li>';
    $all_selected = TRUE;
  }
  else {
    $menu = '<li><a href="' . base_path() . 'admin/structure/block/role">All</a></li>';
  }

  // Role links
  foreach ($roles as $rid => $role) {

    // select role link if rid is current context
    if (block_context_current_rid() == $rid) {
      $menu = $menu . '<li><a class="selected" href="' . base_path() . 'admin/structure/block/role/' . $rid . '">' . $role . '</a></li>';
    }
    // deselect link if path is not current
    else {
      $menu = $menu . '<li><a href="' . base_path() . 'admin/structure/block/role/' . $rid . '">' . $role . '</a></li>';
    }
  }

  return $menu;
}

/**
 * UI Selected Roles Dropdown Menu
 *
 * $block_key       : in the form 'module_delta'
 * $block_roles     : array of roles that this block is enabled for
 *
 * returns          : html dropdown menu for the block
 */
function ui_roles_dropdown($block_key, $block_roles) {

  // define system user roles
  $all_roles = user_roles();

  // create default menu item
  $menu = '<option>All roles</option>';

  // Role ID filter
  if (block_context_current_rid()) {

    // for each system role
    foreach ($all_roles as $rid => $role) {

      // select link if one of block roles matches all_roles
      if ( block_context_block_roles_matches_role($block_roles, $rid) ) {

        $menu = $menu . '<option selected="selected" value="' . $rid . '">' . $role . '</option>';
      }
      // deselect link if path is not current
      else {
        $menu = $menu . '<option value="' . $rid . '">' . $role . '</option>';
      }
    }
  }
  // All filter
  else {

    // for each system role
    foreach ($all_roles as $rid => $role) {

      // select option if this block has roles
      if (!empty($block_roles)) {
        $selected = TRUE;

        $menu = $menu . '<option selected="selected" value="' . $rid . '">' . $role . '</option>';
      }
      // deselect link if path is not current
      else {
        $menu = $menu . '<option value="' . $rid . '">' . $role . '</option>';
      }
    }
  }

  // wrap menu items inside select menu
  $menu ='<select name="blocks['  .  $block_key  .  '][rid]">'.  $menu  .  '</select>';
  return $menu;
}


// NODE TYPES

/**
 * UI Block Context Node types Menu
 *
 * $node_types   : array of node types in the form of key => node_type (0 => page)
 */
function ui_node_types_menu($node_types) {

  // Sellect All link if URL is 'root'
  $all_selected = FALSE;

  // All
  if (block_context_current_node_type() == '') {

    $menu = '<li><a class="selected" href="' . base_path() . 'admin/structure/block/node-type">All</a></li>';
    $all_selected = TRUE;
  }
  else {
    $menu = '<li><a href="' . base_path() . 'admin/structure/block/node-type">All</a></li>';
  }

  // Node types
  foreach ($node_types as $node_type => $node_name) {

    // select node type link if node type is current context
    if (block_context_current_node_type() == $node_type) {
      $menu = $menu . '<li><a class="selected" href="' . base_path() . 'admin/structure/block/node-type/' . $node_type . '">' . $node_name . '</a></li>';
    }
    // deselect link if path is not current
    else {
      $menu = $menu . '<li><a href="' . base_path() . 'admin/structure/block/node-type/' . $node_type . '">' . $node_name . '</a></li>';
    }
  }

  return $menu;
}

/**
 * UI Selected Node types Dropdown Menu
 *
 * $block_key       : in the form 'module_delta'
 * $block_roles     : array of roles that this block is enabled for
 *
 * returns          : html dropdown menu for the block
 */
function ui_node_types_dropdown($block_key, $block_node_types) {

  // define system user roles
  $all_node_types = block_context_node_types();

  // create default menu item
  $menu = '<option>All roles</option>';

  // Role ID filter
  if (block_context_current_node_type()) {

    // for each system role
    foreach ($all_node_types as $node_type => $node_name) {

      // select link if one of block roles matches all_roles
      if ( block_context_block_node_types_matches_node_type($block_node_types, $node_type) ) {

        $menu = $menu . '<option selected="selected" value="' . $node_type . '">' . $node_name . '</option>';
      }
      // deselect link if path is not current
      else {
        $menu = $menu . '<option value="' . $node_type . '">' . $node_name . '</option>';
      }
    }
  }
  // All filter
  else {

    // for each system role
    foreach ($all_node_types as $node_type => $node_name) {

      // select option if this block has roles
      if (!empty($block_roles)) {
        $selected = TRUE;

        $menu = $menu . '<option selected="selected" value="' . $node_type . '">' . $node_name . '</option>';
      }
      // deselect link if path is not current
      else {
        $menu = $menu . '<option value="' . $node_type . '">' . $node_name . '</option>';
      }
    }
  }

  // wrap menu items inside select menu
  $menu ='<select name="blocks['  .  $block_key  .  '][rid]">'.  $menu  .  '</select>';
  return $menu;
}