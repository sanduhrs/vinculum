<?php
/**
 * @file
 * Trackback handler
 *
 * @see http://archive.cweiske.de/trackback/trackback-1.2.html
 *
 * Class handler provides the tools to:
 * - Send a trackback.
 * - Discover if a URL provides trackback, and look up the endpoint URL.
 */
class VinculumTrackbackHandler {

  // User-agent to use when querying remote sites.
  const UA = 'Drupal Trackback (+http://drupal.org/project/vinculum)';

  // Minimum error-level to trigger a watchdog message.
  // Messages below this error-level will not be reported.
  public static $watchdogReportingLevel = WATCHDOG_DEBUG;

  /**
   * Send a Trackback request.
   *
   * @param string $remote_url
   *   The absolute URL of the remote article.
   * @param string $local_url
   *   The absolute URL of the local node.
   * @param array $params
   *   - title
   *     A string title of the local node.
   *   - excerpt
   *     An optional string excerpt referencing the remote post.
   *   - blog_name
   *     A string name of this blog/website (defaults to site-name).
   *
   * @return bool
   *   TRUE if the remote site reported success.
   *   FALSE if the remote site reported failure, or any other error occurred
   *   (e.g., network error preventing the site from being reached).
   */
  public static function send($remote_url, $local_url, $params) {
    // $title, $excerpt = NULL, $blog_name = NULL.
    if ($endpoint = self::getEndpoint($remote_url)) {
      $params['url'] = $local_url;
      return self::sendTrackback($endpoint, $params);
    }
  }

  /**
   * Send a trackback request (notify a third-party site of a link).
   *
   * @param string $endpoint_url
   *   The URL of the remote Trackback endpoint.
   * @param array $params
   *   - url
   *     The URL of the local node (REQUIRED).
   *   - title
   *     The title of the local node (optional).
   *   - excerpt
   *     Excerpt from the local node (optional).
   *   - blog_name
   *     Name of the local site (optional; defaults to site-name).
   *
   * @throws Exception
   * @return bool
   *   TRUE  if the remote site reported success.
   *   FALSE if the remote site reported failure, or any other error occurred
   *   (e.g., network error preventing the site from being reached).
   */
  public static function sendTrackback($endpoint_url, $params) {
    if (!is_array($params) || empty ($params['url'])) {
      throw new Exception("sendTrackback() requires that \$params['url'] is provided");
    }

    // Add default "blog_name" if it's not provided.
    $params += array(
      'blog_name' => variable_get('site_name', 'Drupal'),
    );

    $options = array(
      // The Trackback protocol supports requires POST.
      'method' => 'POST',
      'headers' => array(
        'User-Agent' => self::UA,
        'Content-Type' => 'application/x-www-form-urlencoded; charset=utf-8',
      ),
      // Add the data to send in the Trackback.
      'data' => drupal_http_build_query($params),
    );
    $result = drupal_http_request($endpoint_url, $options);

    // Parse the remote server's response for any error messages.
    if ($result->data) {
      $remote_errors = self::parseRemoteServerResponseForErrors($result->data);

      // Log the error.
      if (is_null($remote_errors)) {
        // The response could not be parsed.
        $params = array(
          '%url' => $endpoint_url,
          '!verbose_output' => filter_xss(str_replace("\n", "<br />\n", htmlentities($result->data))),
        );
        self::debug('Trackback response for endpoint %url could not be parsed as XML.<br />!verbose_output', $params, WATCHDOG_DEBUG);
      }
      elseif (is_string($remote_errors)) {
        $params = array(
          '%url' => $endpoint_url,
          '!verbose_output' => filter_xss(str_replace("\n", "<br />\n", htmlentities($remote_errors))),
        );
        self::debug('Trackback failed for endpoint %url with the message:<br />!verbose_output', $params, WATCHDOG_DEBUG);
      }
      elseif ($remote_errors) {
        $params = array(
          '%url' => $endpoint_url,
        );
        self::debug('Trackback failed for endpoint %url. No error message was provided.', $params, WATCHDOG_DEBUG);
      }

      return ($remote_errors === FALSE);
    }
    return FALSE;
  }

  /**
   * Get the URL of the endpoint that handles trackbacks for a URL.
   *
   * @param string $url
   *   URL of the remote article
   *
   * @return string|FALSE
   *   Absolute URL of the endpoint, or FALSE if trackback is not supported.
   */
  public static function getEndpoint($url) {
    // Efficiency-check: HEAD the URL to lookup the mime-type; skip the full
    // auto-discovery if the content doesn't appear to be text.
    if (!$result = self::urlMimeTypeIsTextBased($url)) {
      // Possible return values from the helper:
      // - TRUE:  content *might* support Trackbacks.
      // - FALSE: content is unlikely to support Trackbacks.
      // - NULL:  remote did not provide a valid response to a HEAD request.
      if (is_null($result)) {
        self::debug('Trackback auto-discovery not attempted: an error occurred when making a HEAD request for %url.', array('%url' => $url), WATCHDOG_DEBUG);
      }
      else {
        self::debug('Trackback auto-discovery not attempted: %url does not appear to be a web page.', array('%url' => $url), WATCHDOG_DEBUG);
      }

      return FALSE;
    }

    // Auto-discovery steps.
    // @see http://www.sixapart.com/pronet/docs/trackback_spec#Auto-Discovery_of_TrackBack_Ping_URLs
    //
    // Given a URL my_url, clients should follow these steps:
    // 1. Send an HTTP GET request to retrieve the contents of the page
    // at my_url.
    // 2. Scan the page contents for embedded RDF. Pages can contain multiple
    // instances of embedded RDF -- clients should pick the block whose
    // dc:identifier matches my_url.
    // 3. Extract the trackback:ping value from the block of RDF.
    // This is the TrackBack Ping URL.

    // Request the third-party URL.
    $options = array(
      'headers' => array(
        'User-Agent' => self::UA,
      ),
    );
    $result = drupal_http_request($url, $options);

    // If the request failed, abort early.
    if (isset($result->error) || $result->code != 200 || empty($result->data)) {
      self::debug('Trackback auto-discovery not attempted: an error occurred when making a GET request for %url.', array('%url' => $url), WATCHDOG_DEBUG);
      return NULL;
    }

    // Test for Trackback endpoint RDF metadata within the page.
    $endpoints = self::discoverEndpointInHTML($result->data);

    // Pages might list multiple endpoints: use the endpoint whose identifier
    // matches the URL provided.
    if (is_null($endpoints)) {
      self::debug('Trackback auto-discovery not checked on %url - the html cannot be parsed.', array('%url' => $url), WATCHDOG_DEBUG);
      return FALSE;
    }
    elseif (!isset($endpoints[$url])) {
      self::debug('Trackback auto-discovery is not supported on %url.', array('%url' => $url), WATCHDOG_DEBUG);
      return FALSE;
    }
    else {
      self::debug('Trackback auto-discovery is supported on %url, with the Trackback endpoint %endpoint.', array('%url' => $url, '%endpoint' => $endpoints[$url]), WATCHDOG_DEBUG);
      return $endpoints[$url];
    }
  }

  /**
   * Get the trackback-structured RDF metadata to provide auto-discovery.
   *
   * @param string $local_url
   *   The absolute URL of the local page or node.
   * @param string $endpoint_url
   *   The trackback-handling endpoint for registering trackbacks using an XML
   *   call. For nodes, this is node/$nid/trackback.
   * @param string $title
   *   The title of the page/node.
   *
   * @return string
   */
  public static function buildTrackbackRDF($local_url, $endpoint_url, $title) {
    $rdf_text = "\n<!--\n";
    $rdf_text .= '<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:trackback="http://madskills.com/public/xml/rss/module/trackback/">' . "\n";
    $rdf_text .= "\t" . '<rdf:Description';
    $rdf_text .= ' rdf:about="' . $local_url . '"';
    $rdf_text .= ' dc:identifier="' . $local_url . '"';
    $rdf_text .= ' dc:title="' . strtr(check_plain($title), array('--' => '&mdash;')) . '"';
    $rdf_text .= ' trackback:ping="' . $endpoint_url . '" />' . "\n";
    $rdf_text .= '</rdf:RDF>';
    $rdf_text .= "\n-->\n";

    return $rdf_text;
  }

  /**
   * The response expected if a trackback succeeded.
   */
  public static function getResponseSuccess() {
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
   * The response expected if a trackback failed.
   */
  public static function getResponseFailed($message = '') {
    // The Trackback standard's response format for failed requests.
    $response = array();

    $response[] = '<?xml version="1.0" encoding="utf-8"?>';
    $response[] = '<response>';
    $response[] = '<error>1</error>';
    $response[] = '<message>' . check_plain($message) . '</message>';
    $response[] = '</response>';

    // Add a trailing new-line.
    $response[] = '';

    return implode("\n", $response);
  }

  /**
   * The response expected if a trackback failed.
   *
   * @param string $failure_message
   *   Human-readable message to provide to the remote server to describe the
   *   error. For example "Trackback already registered".
   *
   * @return string
   */
  public static function getResponseFailure($failure_message = NULL) {
    if (is_null($failure_message)) {
      // Default message is *untranslated*.
      $failure_message = 'An unknown error occurred';
    }

    // The Trackback standard's response format for failed requests.
    $response = array();
    $response[] = '<?xml version="1.0" encoding="utf-8"?>';
    $response[] = '<response>';
    $response[] = '<error>1</error>';
    $response[] = '<message>' . htmlentities($failure_message) . '</message>';
    $response[] = '</response>';
    // Add a trailing new-line.
    $response[] = '';

    return implode("\n", $response);
  }

  /**
   * HEAD the URL and inspect mime-type to see if it appears to be text-based.
   *
   * @param string $url
   *   URL of the remote article.
   *
   * @return bool|NULL
   *   - TRUE
   *     if the URL was reached and might support Trackback auto-discovery.
   *   - FALSE
   *     if the URL was reached and is unlikely to support Trackback
   *     auto-discovery.
   *   - NULL
   *     if the URL could not be reached (for example, due to a 404 error).
   */
  public static function urlMimeTypeIsTextBased($url) {
    $skip_test = variable_get('vinculum_trackback_autodiscovery_skip_efficiency_check', FALSE);
    if ($skip_test) {
      return TRUE;
    }

    $options = array(
      'method' => 'HEAD',
      'headers' => array(
        'User-Agent' => self::UA,
      ),
    );
    $result = drupal_http_request($url, $options);

    // Errors or any non-200 response fails.
    if (isset($result->error) || $result->code != 200) {
      return NULL;
    }

    $mime_type = (isset($result->headers['content-type'])) ? $result->headers['content-type'] : NULL;
    $is_valid = substr_count($mime_type, 'text/html')
      || substr_count($mime_type, 'application/xhtml+xml')
      || substr_count($mime_type, 'application/xml')
      || substr_count($mime_type, 'text/xml');
    return $is_valid;
  }

  /**
   * Extract the Trackback endpoints from a section of text.
   *
   * @param string $html
   *   The HTML to search
   *
   * @return array|NULL
   *   Array of discovered trackback endpoints (where the key is the entity's
   *   identifier - i.e. the remote page's URL - and the value is the endpoint).
   *   If no Trackback endpoints are discovered, an empty array will be
   *   returned. If the html cannot be parsed, NULL is returned.
   */
  public static function discoverEndpointInHTML($html) {
    $matches = array();

    // We need to match: RDF/Description[@ping and @identifier]

    // If RDF tags are wrapped in comment tags, remove the comment tags so the
    // RDF tags can be parsed by SimpleXML.
    $html = trim(self::stripCommentTags($html));

    $xml = @simplexml_load_string($html) or
    $xml = @simplexml_load_string(self::cleanupHTML($html));
    if ($xml === FALSE) {
      // If the html can't be loaded by simplexml, abort.
      return NULL;
    }

    // Discover three unused namespace prefixes, to avoid namespace prefix
    // collision.

    // (There's an issue in Simplexml which prevents namespace-prefixed access
    // to nodes which redefine a pre-existing namespace prefix to a new
    // namespace - the node can be accessed by the correct namespace with a non-
    // conflicting prefix, but cannot be accessed through registering the pre-
    // existing namespace prefix to the correct namespace. SimpleXML WTF.
    list($p1, $p2, $p3) = self::generateUniqueXmlPrefixes($xml->getNamespaces(TRUE));

    $xml->registerXPathNamespace($p1, 'http://www.w3.org/1999/02/22-rdf-syntax-ns#');
    $xml->registerXPathNamespace($p2, 'http://purl.org/dc/elements/1.1/');
    $xml->registerXPathNamespace($p3, 'http://madskills.com/public/xml/rss/module/trackback/');
    if ($rdf_tags = $xml->xpath("//{$p1}:RDF/{$p1}:Description[@{$p2}:identifier and @{$p3}:ping]")) {
      foreach ($rdf_tags as $element) {
        $identifier = (string) $element->attributes('http://purl.org/dc/elements/1.1/')->identifier;
        $ping = (string) $element->attributes('http://madskills.com/public/xml/rss/module/trackback/')->ping;
        $matches[$identifier] = $ping;
      }
    }

    // The last resort.
    if (empty($matches)) {
      preg_match_all('/dc:identifier="(.*)"/', self::cleanupHTML($html), $identifier_matches, PREG_PATTERN_ORDER);
      if (!empty($identifier_matches)) {
        $identifier = array_pop($identifier_matches)[0];
      }
      preg_match_all('/trackback:ping="(.*)"/', self::cleanupHTML($html), $ping_matches, PREG_PATTERN_ORDER);
      if (!empty($ping_matches)) {
        $ping = array_pop($ping_matches)[0];
        $matches[$identifier] = $ping;
      }
    }

    return $matches;
  }

  /**
   * Where RDF tags are wrapped by comment tags, remove the comment tags.
   */
  public static function stripCommentTags($html) {
    // If RDF tags are found within a comment section, strip out the comment
    // tags so the RDF can be parsed via SimpleXml.

    // Simply run every comment tag through a helper function.
    // The /m modifier indicates multi-line matching, and /s allows the dot [.]
    // character to include \n (the new-line character).
    // The ? character (in .*?) indicated non-greedy matching.
    $html = preg_replace_callback('/<!--(.*?)-->/ms',
      create_function(
        '$matches',
        'return preg_match("#<([^:]*:)?RDF\s#ms", $matches[1]) ? $matches[1] : "<!--" . $matches[1] . "-->";'
      ),
      $html);
    return $html;
  }

  /**
   * Attempt to clean HTML to allow SimpleXML to load the HTML.
   */
  public static function cleanupHTML($html) {
    // Strip everything except the RDF tags.
    $pattern = '#<rdf:RDF (?:.*?)</rdf:RDF>#ms';
    if (preg_match($pattern, $html, $matches)) {
      $res = "<?xml version='1.0'?>";
      $res .= implode("\n", $matches);
      return $res;
    }

    // Final attempt: Run it through the Drupal filter HTML corrector.
    return trim(_filter_htmlcorrector($html));
  }

  /**
   * Generate 3 unique namespace prefixes.
   *
   * To avoid collision with existing namespace prefixes.
   *
   * @param array $namespaces
   *   Array of current namespaces in use (see SimpleXMLElement::getNamespaces).
   *
   * @return array
   *   An array of 3 unique 5-character namespace prefixes.
   */
  public static function generateUniqueXmlPrefixes($namespaces) {
    // Each new prefix will be a random 5-character string using lower-case
    // alphabetic characters.
    $character_set = 'abcdefghijklmnopqrstuvwxyz';

    $new_prefixes = array();
    $prefixes = array_keys($namespaces);
    while (count($new_prefixes) < 3) {
      $prefix = $character_set[rand(0, 25)]
              . $character_set[rand(0, 25)]
              . $character_set[rand(0, 25)]
              . $character_set[rand(0, 25)]
              . $character_set[rand(0, 25)];
      if (!(in_array($prefix, $prefixes) || in_array($prefix, $new_prefixes))) {
        $new_prefixes[] = $prefix;
      }
    }
    return $new_prefixes;
  }

  /**
   * Parse the response from a trackback endpoint.
   *
   * And report if the response is an error (and any message, if provided).
   *
   * @param string $html
   *   The response provided by the Trackback endpoint.
   *
   * @return mixed
   *   - NULL
   *     if the HTML cannot be parsed (is not valid XML)
   *   - FALSE
   *     if no errors could be found
   *   - TRUE
   *     if errors were found but no error message provided
   *   - String
   *     the error message provided
   */
  public static function parseRemoteServerResponseForErrors($html) {
    // If the html can't be parsed as XML, return error response NULL.
    if (!$xml = @simplexml_load_string($html)) {
      return NULL;
    }

    // Check for the <error> response.
    $error_response = $xml->xpath('//response/error/text()');
    if (empty($error_response)) {
      return NULL;
    }
    elseif ($error_response[0] == 1) {
      // Check for an error message.
      $error_message = $xml->xpath('//response/message/text()');
      if (count($error_message)) {
        return $error_message[0];
      }
      return TRUE;
    }
    elseif ($error_response[0] == 0) {
      // Error response 0 means OK.
      return FALSE;
    }
    else {
      // There's an <error> response, but it doesn't conform to the standard
      // of being 0 or 1.
      return NULL;
    }
  }

  /**
   * Delegated handler for debug messages.
   *
   * Allowing control over which are passed to watchdog().
   */
  public static function debug($message, $params = array(), $error_level = WATCHDOG_NOTICE) {
    if ($error_level <= self::$watchdogReportingLevel) {
      watchdog('Trackback', $message, $params, $error_level);
    }
  }
}
