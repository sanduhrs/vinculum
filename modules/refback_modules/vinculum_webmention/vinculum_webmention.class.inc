<?php
/**
 * @file
 * Webmention handler
 *
 * @see http://webmention.org
 */

class VinculumWebmentionHandler {

  // User-agent to use when querying remote sites.
  const UA = 'Drupal Webmention (+http://drupal.org/project/vinculum)';

  // Minimum error-level to trigger a watchdog message.
  // Messages below this error-level will not be reported.
  public static $watchdog_reporting_level = WATCHDOG_DEBUG;

  /**
   * Attempt to send a Webmention.
   *
   * @param string $target
   *   Absolute URL of the remote article.
   * @param string $source
   *   Absolute URL of the local page.
   *
   * @return bool
   */
  public static function send($source, $target) {
    //if ($endpoint_url == self::discover($target)) { // should be two equals signs eh
    $params = array(
      '%source' => $source,
      '%target' => $target,
    );

    self::debug('Trying Webmention to %target from %source.', $params, WATCHDOG_DEBUG);

    $methods = array(
      'webmention.webmention' => array($source, $target),
    );
    $response = drupal_http_request($target, array(
      'method' => 'POST',
      'headers' => array(
        'User-Agent' => self::UA,
        'Content-Type' => 'application/x-www-form-urlencoded; charset=utf-8',
      ),
      'data' => drupal_http_build_query($params),
    ));

    if ($response->code === '200') {
      self::debug('Webmention to %target from %source succeeded.', $params, WATCHDOG_INFO);
      return TRUE;
    }
    else {
      $params = array(
        '%source' => $source,
        '%target' => $target,
        '%responsecode' => $response->code,
        '%status_message' => $response->status_message,
        //'@response' => $response,
      );
      dpm($response);
      if ($response->code != '200') {
        drupal_set_message(t('Error: tried to Webmention @target, %responsecode %status_message', 
          array('@target' => $target, '%status_message' => $response->status_message, '%responsecode' => $response->code,)), 'error');
      }
      self::debug('Webmention to %target from %source got different: %responsecode %status_message' , $params, WATCHDOG_INFO);
    }
  
    //else {
    //  self::debug('there was no target %target', $params, WATCHDOG_INFO);
    //}
    return FALSE;
  }

  /**
   * Discover Webmention endpoint.
   *
   * @param string $target
   *   Absolute URL of the remote article.
   *
   * @return mixed
   *   Return endpoint URL as string or boolean FALSE.
   */
  public static function discover($target) {
    $response = drupal_http_request($target);

    if ($response->code != '200') {
      return FALSE;
    }

    // Check for Webmention endpoint advertised in response headers.
    if (array_key_exists('link', $response->headers)) {
      // Convert to array no matter what we have.
      $link_headers = $response->headers['link'];
      if (!is_array($link_headers)) {
        $link_headers = explode(', ', $link_headers);
      }

      foreach ($link_headers as $link_header) {
        if (preg_match('~<((?:https?://)?[^>]+)>; rel="webmention"~', $link_header, $match)) {
          $endpoint = self::absoluteURL($target, array_pop($match));
          return $endpoint;
        }
        elseif (preg_match('~<((?:https?://)?[^>]+)>; rel="http://webmention.org/?"~', $link_header, $match)) {
          $endpoint = self::absoluteURL($target, array_pop($match));
          return $endpoint;
        }
      }
    }

    // Check for <link>- or <a>-element with rel="webmention".
    if (preg_match('/<(?:link|a)[ ]+href="([^"]+)"[ ]+rel="webmention"[ ]*\/?>/i', $response->data, $match) ||
      preg_match('/<(?:link|a)[ ]+rel="webmention"[ ]+href="([^"]+)"[ ]*\/?>/i', $response->data, $match)) {
      return self::absoluteURL($target, array_pop($match));
    }
    elseif (preg_match('/<(?:link|a)[ ]+href="([^"]+)"[ ]+rel="http:\/\/webmention\.org\/?"[ ]*\/?>/i', $body, $match) ||
      preg_match('/<(?:link|a)[ ]+rel="http:\/\/webmention\.org\/?"[ ]+href="([^"]+)"[ ]*\/?>/i', $body, $match)) {
      return self::absoluteURL($target, array_pop($match));
    }

    // No match if we arrive here.
    return FALSE;
  }

  /**
   * Absolutize a URL.
   *
   * @return mixed
   *   A absolute URL as string or boolean FALSE.
   */
  public static function absoluteURL($target, $url) {
    if (substr($url, 0, 4) !== 'http') {
      $parts = parse_url($target);
      $url = $parts['scheme'] . '://' . $parts['host'] . '/' . ltrim($url, '/');
    }

    if (filter_var($url, FILTER_VALIDATE_URL)) {
      return $url;
    }

    return FALSE;
  }

  /**
   * Validate if a source exists.
   *
   * @param $target
   *
   * @return bool
   */
  public static function validateSourceExists($target) {
    return TRUE;
  }

  /**
   * Validate if a source contains target.
   *
   * @param $target
   *
   * @return bool
   */
  public static function validateSourceContainsTarget($source, $target) {
    return TRUE;
  }

  /**
   * Delegated handler for debug messages, allowing control over which are
   * passed to watchdog().
   */
  public static function debug($message, $params = array(), $error_level = WATCHDOG_NOTICE) {
    if ($error_level <= self::$watchdog_reporting_level) {
      watchdog('Webmention', $message, $params, $error_level);
    }
  }

}
