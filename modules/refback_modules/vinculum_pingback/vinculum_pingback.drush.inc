<?php
/**
 * Drush integration for pingback.
 */


/**
 * Implements hook_drush_command().
 */
function vinculum_pingback_drush_command() {

  $items = array();
  $items['vinculum-send-pingback'] = array(
    'callback' => 'vinculum_pingback__drush_send_pingback',
    'description' => "Send a pingback.",
    'arguments' => array(
      'nid' => "The node ID that the pingback comes from.",
      'target' => "The URL to send the pingback to.",
    ),
    'examples' => array(
      'drush vinculum-send-pingback 45 http://example.com/blog/2011/11/09/foo' =>
        "Send a pingback from node 45 to http://example.com/blog/2011/11/09/foo.",
    ),
  );

  $items['vinculum-receive-pingback'] = array(
    'callback' => 'vinculum_pingback__drush_receive_pingback',
    'description' => "Receive a pingback.",
    'arguments' => array(
      'nid' => "The node ID that the pingback is targetting.",
      'source' => "The URL that sent the pingback.",
    ),
    'examples' => array(
      'drush vinculum-receive-pingback 45 http://example.com/blog/2011/11/09/foo' =>
        "Receive a pingback on node 45 from http://example.com/blog/2011/11/09/foo.",
    ),
  );

  return $items;
}


/**
 * Send a pingback through drush.
 */
function vinculum_pingback__drush_send_pingback($nid, $target) {
  $node = node_load($nid);
  if (!$node) {
    drush_set_error('DRUSH_NOT_COMPLETED', "Node not found.");
  }
  else {
    $source = url("node/{$node->nid}", array('absolute' => TRUE));
    VinculumPingbackHandler::send($source, $target);
  }
}


/**
 * Receive a pingback through drush.
 */
function vinculum_pingback__drush_receive_pingback($nid, $source) {
  $node = node_load($nid);
  if (!$node) {
    drush_set_error('DRUSH_NOT_COMPLETED', "Node not found.");
  }
  else {
    $target = url("node/{$node->nid}", array('absolute' => TRUE));
    vinculum_pingback__receive_pingback($source, $target);
  }
}
