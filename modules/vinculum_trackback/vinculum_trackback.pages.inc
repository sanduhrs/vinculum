<?php
/**
 * @file
 */

/**
 * Receive a trackback request.
 * This page-callback returns an XML response.
 */
function linkback_trackback_receive($node) {

  // Validate the incoming trackback data.
  if (!_linkback_trackback_is_request_valid()) {
    // Log the failure.
    if ($_SERVER['REQUEST_METHOD'] == 'POST') {
      $err = 'The remote server did not provide the URL of the trackback.';
    }
    else if (isset($_REQUEST['url'])) {
      $err = 'The request method was %method instead of POST.';
    }
    else {
      $err = 'The remote server did not provide the URL of the trackback, and the request method was %method instead of POST.';
    }
    $params = array(
      '%method' => $_SERVER['REQUEST_METHOD'],
      '!node' => l($node->title, "node/{$node->nid}"),
    );
    watchdog('Trackback', 'Received an invalid trackback request for !node.' . $err, $params, WATCHDOG_DEBUG);

    // Abort early for invalid requests.
    $msg = t('Malformed Trackback request: requests must use POST and provide a url.  See http://www.sixapart.com/pronet/docs/trackback_spec#Sending_a_TrackBack_Ping');
    $response = _linkback_trackback_failure_text($msg);
    return _linkback_trackback_response($response, TRUE);
  }

  $trackback = _linkback_trackback_build_trackback($node->nid);
  // Attempt to save the trackback.
  // A non-zero value for $result indicates an error.
  $result = linkback_trackback_save_trackback($trackback);

  switch ($result) {
    case LINKBACK_VALIDATES:
      // Report success.
      watchdog('Trackback', 'Received a successful trackback request for !node.', array('!node' => l($node->title, "node/{$node->nid}")), WATCHDOG_DEBUG);
      $response = _linkback_trackback_success_text();
      return _linkback_trackback_response($response);
      break;

    // case LINKBACK_ERROR_LOCAL_NODE_NOT_FOUND:
    //   wouldn't reach this step (it would be 404'd in hook_menu).

    case LINKBACK_ERROR_LOCAL_NODE_LINKBACK_NOT_ALLOWED:
      watchdog('Trackback', 'Trackback request failed for !node: trackback-receive not enabled.', array('!node' => l($node->title, "node/{$node->nid}")), WATCHDOG_DEBUG);
      $msg = t('Trackbacks are not allowed for this page.');
      break;

    case LINKBACK_ERROR_LINKBACK_ALREADY_REGISTERED:
      watchdog('Trackback', 'Trackback request failed for !node: trackback previously registered.', array('!node' => l($node->title, "node/{$node->nid}")), WATCHDOG_DEBUG);
      $msg = t('The Trackback has already been registered.');
      break;

    case LINKBACK_ERROR_REMOTE_URL_NOT_FOUND:
      watchdog('Trackback', 'Trackback request failed for !node: trackback URL %remote_url not accessible.', array('%remote_url' => $trackback->url, '!node' => l($node->title, "node/{$node->nid}")), WATCHDOG_DEBUG);
      $msg = t('Your page @remote_url could not be reached.', array('@remote_url' => $trackback->url));
      break;

    case LINKBACK_ERROR_REMOTE_URL_MISSING_LINK:
      watchdog('Trackback', 'Trackback request failed for !node: trackback URL %remote_url does not link to the node.', array('%remote_url' => $trackback->url, '!node' => l($node->title, "node/{$node->nid}")), WATCHDOG_DEBUG);
      $msg = t('Your page @remote_url must include a link to the page on this site @local_url.', array('@remote_url' => $trackback->url, '@local_url' => url("node/{$node->nid}", array('absolute' => TRUE))));
      break;

    default:
      watchdog('Trackback', 'Trackback request failed for !node: unknown error.', array('!node' => l($node->title, "node/{$node->nid}")), WATCHDOG_DEBUG);
      $msg = t('An unknown error occurred.');
      break;
  }
  $response = _linkback_trackback_failure_text($msg);
  return _linkback_trackback_response($response, TRUE);
}

/**
 * Display the response.
 */
function _linkback_trackback_response($response, $error = FALSE) {
  drupal_add_http_header('Content-Type', 'text/xml');
  if ($error) {
    // Provide a generic HTTP error code.
    drupal_add_http_header('Status', '400 Bad Request');
  }
  else {
    // If a Trackback is added, 202 (Accepted) is the most appropriate response
    // (because the Trackback resource may not have its own URL, and moderation
    // may subsequently decline the request).
    drupal_add_http_header('Status', '202 Accepted');
  }

  echo $response;
  return NULL;
}

/**
 * Get the text for reporting a successful Trackback request.
 */
function _linkback_trackback_success_text() {
  // The Trackback standard's response format for successful requests.
  $response = array();
  $response[] = '<?xml version="1.0" encoding="utf-8"?>';
  $response[] = '<response>';
  $response[] = '<error>0</error>';
  $response[] = '</response>';
  // Add a trailing new-line.
  $response[] = '';

  return implode("\n", $response);
}

/**
 * Get the text for reporting a failed Trackback request.
 */
function _linkback_trackback_failure_text($msg) {
  // The Trackback standard's response format for failed requests.
  $response = array();
  $response[] = '<?xml version="1.0" encoding="utf-8"?>';
  $response[] = '<response>';
  $response[] = '<error>1</error>';
  $response[] = '<message>' . htmlentities($msg) . '</message>';
  $response[] = '</response>';
  // Add a trailing new-line.
  $response[] = '';


  return implode("\n", $response);
}

/**
 * Validate the parameters of a Trackback submission.
 */
function _linkback_trackback_is_request_valid() {
  $is_valid = $_SERVER['REQUEST_METHOD'] == 'POST' && !empty($_REQUEST['url']);
  return $is_valid;
}

/**
 * Build a trackback object with the parameters provided in the request.
 */
function _linkback_trackback_build_trackback($nid) {
  // Trackback requests should contain a number of parameters, provided as POST
  // variables.
  $trackback = new stdClass;
  $trackback->nid = (int) $nid;
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

  return $trackback;
}
