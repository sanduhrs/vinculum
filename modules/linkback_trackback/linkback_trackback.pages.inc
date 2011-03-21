<?php
/**
 * @file
 */

/**
 * Receive a trackback request.
 */
function linkback_trackback_receive($node) {

  // Trackback requests should contain a number of parameters, provided either
  // as GET or POST variables.
  $trackback = new stdClass;
  $trackback->nid = (int) $node->nid;
  $trackback->origin_ip = ip_address();
  $trackback->timestamp = time();

  // Add the fields defined in the trackback protocol.
  $allowed_remote_data = array(
    'url',
    'blog_name',
    'title',
    'excerpt',
  );
  foreach ($allowed_remote_data as $variable) {
    $trackback->$variable = isset($_REQUEST[$variable]) ? $_REQUEST[$variable] : NULL;
  }
  // @TODO: do these parameters need cleaning / UTF8 translation?

  // Attempt to save the trackback.  Non-zero results indicate an error.
  if ($result = linkback_trackback_save_trackback($trackback)) {
    // Provide a generic HTTP error code.
    drupal_add_http_header('Status', '400 Bad Request');

    switch ($result) {
      // case LINKBACK_ERROR_LOCAL_NODE_NOT_FOUND: wouldn't reach this step
      // (it would be 404'd in hook_menu).

      case LINKBACK_ERROR_LOCAL_NODE_LINKBACK_NOT_ALLOWED:
        $msg = t('Trackbacks are not allowed for this page.');
        break;

      case LINKBACK_ERROR_LINKBACK_ALREADY_REGISTERED:
        $msg = t('The Trackback has already been registered.');
        break;

      case LINKBACK_ERROR_REMOTE_URL_NOT_FOUND:
        $msg = t('Your page @remote_url could not be reached.', array('@remote_url' => $trackback->url));
        break;

      case LINKBACK_ERROR_REMOTE_URL_MISSING_LINK:
        $msg = t('Your page @remote_url must include a link to the page on this site @local_url.', array('@remote_url' => $trackback->url, '@local_url' => url("node/{$node->nid}", array('absolute' => TRUE))));
        break;

      default:
        $msg = t('An unknown error occurred.');
        break;
    }
  }
  else {
    $msg = t('Trackback has been saved.');
  }

  // Output the message and return NULL (the URL is designed for automated
  // requests, so a plain-text message will be more readable than a fully-
  // themed page).
  echo $msg;
  return NULL;
}
