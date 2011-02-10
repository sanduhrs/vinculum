<?php
/**
 * @file
 * Hooks provided by the linkback module.
 */

/**
 * @defgroup linkback_api_hooks Linkback API Hooks
 * @{
 * Functions to integrate with the Linkback module.
 * @}
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Declare a linkback handler.
 *
 * This is a hook used by linkback handler modules. It allows a module to 
 * define a handler which implements a linkback protocol.
 *
 * @return
 *   An array containing the key 'protocol' which provides the human-readable
 *   name of the protocol implemented.  A linkback module can only implement a
 *   single handler.
 *  
 * @ingroup linkback_api_hooks
 */
function hook_linkback_handler() {
  return array(
    'protocol' => t('Pingback'),
  );
}

/**
 * Attempt to send a linkback to a single URL for a single node.
 *
 * @param String $target
 *    The URL of the external site. You can assume that the link will always be
 *    a FQDN.
 * @param String $source
 *    The canonical URL of the local node.
 * @param Object $node
 *    The node that links to the URL provided.
 *
 * @return Boolean
 * TRUE if the handler was successful in reporting the linkback.
 */
function hook_linkback_send($target, $source, $node) {
  
}

/**
 * Extend the linkback module to check for external links in fields other than
 * the body field.
 */
function hook_linkback_get_external_links($node) {
  
}

/**
 * Alter the list of external-links to be contacted by a linkback handler.
 *
 * @param Array $links
 * An array of linkback records: each record is an object with the properties:
 * - source
 * - target
 */
function hook_linkback_link_send_alter(&$links) {
  // Don't attempt linkbacks on example.com domains.
  foreach($links as $key => $record) {
    if (preg_match('#\.example\.com$#', $record->target)) {
      unset ($links[$key]);
    }
  }
}

/**
 * Actions to be fired when a linkback is received.
 */
function hook_linkback_received_action($record) {
  
}

/**
 * @} End of "addtogroup hooks".
 */
