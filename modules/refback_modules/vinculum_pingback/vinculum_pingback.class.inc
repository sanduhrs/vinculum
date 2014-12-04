<?php
/**
 * @file
 * Pingback handler
 *
 * @see http://www.hixie.ch/specs/pingback/pingback-1.0
 * @see http://www.w3.org/wiki/Pingback
 *
 * Class handler provides the tools to:
 * - Send a Pingback.
 * - Discover if a URL provides Pingback, and look up the XML-RPC endpoint.
 */

class VinculumPingbackHandler {

  // User-agent to use when querying remote sites.
  const UA = 'Drupal Pingback (+http://drupal.org/project/vinculum)';

  // Minimum error-level to trigger a watchdog message.
  // Messages below this error-level will not be reported.
  public static $watchdog_reporting_level = WATCHDOG_DEBUG;

  /**
   * Attempt to send a pingback.  The pingback protocol uses a simple static
   * schema: only the origin and target URLs are used.
   *
   * @param String $target
   * Absolute URL of the remote article.
   * @param String $source
   * Absolute URL of the local page.
   *
   * @return bool
   */
  public static function send($source, $target) {
    // Check whether the remote target supports pingbacks.
    if ($xmlrpc_endpoint = self::getXmlRpcEndpoint($target)) {
      $params = array(
        '%source' => $source,
        '%target' => $target,
        '%endpoint' => $xmlrpc_endpoint,
      );
      self::debug('Attempting pingback to %target from %source via endpoint %endpoint.', $params, WATCHDOG_DEBUG);

      $methods = array(
        'pingback.ping' => array($source, $target),
      );
      // Send the XML-RPC request.
      $result = xmlrpc($xmlrpc_endpoint, $methods, array('headers' => array('User-Agent' => self::UA)));

      if ($result) {
        $params = array(
          '%source' => $source,
          '%target' => $target,
        );
        self::debug('Pingback to %target from %source succeeded.', $params, WATCHDOG_INFO);
        return TRUE;
      }
      else {
        $params = array(
          '%source' => $source,
          '%target' => $target,
          '@errno' => xmlrpc_errno(),
          '@description' => xmlrpc_error_msg(),
        );
        self::debug('Pingback to %target from %source failed.<br />Error @errno: @description', $params, WATCHDOG_WARNING);
        return FALSE;
      }
    }
    // No XML-RPC endpoint detected; pingback failed.
    return FALSE;
  }

  /**
   * Get the URL of the XML-RPC endpoint that handles pingbacks for a URL.
   *
   * @param String $url
   * URL of the remote article
   *
   * @return String|FALSE
   * Absolute URL of the XML-RPC endpoint, or FALSE if pingback is not
   * supported.
   */
  public static function getXmlRpcEndpoint($url) {
    self::debug('Attempting pingback auto-discovery on %url.', array('%url' => $url), WATCHDOG_DEBUG);

    // 1.  Check for an X-Pingback header.
    $result = drupal_http_request($url, array(
      'method' => 'HEAD',
      'headers' => array('User-Agent' => self::UA),
    ));

    // NB: all headers from drupal_http_request are changed to lower-case.
    if (empty($result->error) && is_array($result->headers) && isset($result->headers['x-pingback'])) {
      $endpoint = check_url($result->headers['x-pingback']);
      self::debug('Pingback auto-discovery is supported on %url, with the Pingback endpoint %endpoint.<br />(using X-PINGBACK HTTP header).', array('%url' => $url, '%endpoint' => $endpoint), WATCHDOG_DEBUG);
      return $endpoint;
    }

    // 2.  No X-PINGBACK header, so do a full request for the page, check for a
    // <link> tag in the <head> section.
    elseif (empty($result->error)) {
      $result = drupal_http_request($url, array(
        'method' => 'GET',
        'headers' => array('User-Agent' => self::UA),
      ));

      if (empty($result->error)) {
        // This regexp is the one provided in the spec:
        // http://www.hixie.ch/specs/pingback/pingback-1.0
        if (preg_match('#<link rel="pingback" href="([^"]+)" ?/?>#', $result->data, $matches)) {
          $endpoint = check_url($matches[1]);
          self::debug('Pingback auto-discovery is supported on %url, with the Pingback endpoint %endpoint.<br />(using &lt;link rel="pingback"&gt; tag).', array('%url' => $url, '%endpoint' => $endpoint), WATCHDOG_DEBUG);
          return $endpoint;
        }
      }
    }

    // No X-Pingback header, no <link rel="pingback">.
    self::debug('Pingback auto-discovery is not supported on %url.', array('%url' => $url), WATCHDOG_DEBUG);
    return FALSE;
  }

  /**
   * Delegated handler for debug messages, allowing control over which are
   * passed to watchdog().
   */
  public static function debug($message, $params = array(), $error_level = WATCHDOG_NOTICE) {
    if ($error_level <= self::$watchdog_reporting_level) {
      watchdog('Pingback', $message, $params, $error_level);
    }
  }
}
