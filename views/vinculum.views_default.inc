<?php
/**
 * @file
 * Default views.
 */

/**
 * Implements hook_views_default_views().
 */
function vinculum_views_default_views() {
  $views = array();

  // Parse each .view file in the views folder.
  $path = dirname(__FILE__) . '/exported_views';
  foreach (glob("{$path}/*.view.inc") as $file) {
    include($file);
    $views[$view->name] = $view;
  }
  return $views;
}
