<?php
/**
 * @file
 * The Vinculum entity class.
 */

/**
 * Entity class for received ref-backs.
 */
class VinculumReceived extends Entity {

  /**
   * Entity properties about all received ref-backs:
   * - Node NID
   * - URL of the article which linked to us
   * - Title   (extracted from the <title> tag)
   * - Excerpt (extracted from the body content)
   * - IP address
   * - Timestamp
   * - Handler
   */
  public $nid;
  public $url;
  public $title;
  public $excerpt;
  public $ip_address;
  public $created;
  public $handler;


  /**
   * Constructor for new received ref-back entities.
   *
   * @param array $values
   * Array with the keys:
   * - handler         REQUIRED
   * - nid OR localUrl REQUIRED
   * - url             REQUIRED
   * - title           Defaults to NULL
   * - excerpt         Defaults to NULL
   * - ip_address      Defaults to ip_address()
   * - created       Defaults to time()
   */
  public function __construct(array $values = array(), $entityType = NULL) {
    // If localUrl is provided, attempt to convert to a node NID.
    if (empty($values['nid']) && isset($values['localUrl'])) {
      $values['nid'] = self::lookupNid($values['localUrl']);
      $this->nidSource = 'url';
    }

    $defaults = array(
      'handler'    => NULL,
      'nid'        => NULL,
      'url'        => NULL,
      'title'      => NULL,
      'excerpt'    => NULL,
      'ip_address' => ip_address(),
      'created'  => REQUEST_TIME,
    );
    foreach ($defaults as $key => $default) {
      $this->$key = (isset($values[$key])) ? $values[$key] : $defaults[$key];
      unset($values[$key]);
    }


    parent::__construct($values, 'vinculum_received');
  }

  /**
   * Given a local URL, attempt to lookup its NID.
   *
   * @param String $url
   * The URL to a local page.
   *
   * @return Int|FALSE
   * The node NID, or FALSE if a corresponding node cannot be found.
   */
  protected static function lookupNid($url) {
    $path = str_replace($GLOBALS['base_url'] . "/" , "", $url);
    if ($path != $url) {
      $node_path = explode('/', drupal_get_normal_path($path));
      if ($node_path[0] == 'node' && is_numeric($node_path[1]) && !isset($node_path[2]) ) {
        return (int)$node_path[1];
      }
    }
    return FALSE;
  }

  /**
   * Gets an excerpt of the source site.
   * Adapted from Pingback Drupal 6 module.
   *
   * @param String $pagelinkedfrom
   *   The URL of the source site.
   * @param String $pagelinkedto
   *   The Url of the target site.
   * @return
   *   An array with title and excerpt or FALSE in case of problems.
   */
  protected static function vinculumGetExcerpt($pagelinkedfrom, $pagelinkedto) {
    $excerpt = FALSE;
    // Let's check the remote site
    $r = drupal_http_request($pagelinkedfrom, array('headers' => array('User-Agent' => VINCULUM_UA)));
    if (!empty($r->error) and !$response->code == 200) {
      return FALSE;
    }
    $linea = $r->data;
    // Work around bug in strip_tags():
    $linea = str_replace('<!DOC', '<DOC', $linea);
    $linea = preg_replace('/[\s\r\n\t]+/', ' ', $linea); // normalize spaces
    $linea = preg_replace('/ <(h1|h2|h3|h4|h5|h6|p|th|td|li|dt|dd|pre|caption|input|textarea|button|body|b
          r|div|nav)[^>]*>/', "\n\n", $linea);
    preg_match('|<title>([^<]*?)</title>|is', $linea, $matchtitle);
    $title = check_plain($matchtitle[1]);
    if (empty($title)) {
      return FALSE;
    }

    $linea = preg_replace('#<script(.*?)>(.*?)</script>#is', '', $linea); //remove scripts
    $linea = strip_tags($linea, '<a>'); // just keep the tag we need

    $p = explode("\n\n", $linea);

    $preg_target = preg_quote($pagelinkedto);

    foreach ($p as $para) {
      if (strpos($para, $pagelinkedto) !== FALSE) { // it exists, but is it a link?
        preg_match('|<a[^>]+?' . $preg_target . '[^>]*>([^>]+?)</a>|', $para, $context);

        // If the URL isn't in a link context, keep looking
        if (empty($context)) {
          continue;
        }

        // We're going to use this fake tag to mark the context in a bit
        // the marker is needed in case the link text appears more than once in the paragraph
        //I edited <wpcontext></wpcontext> to <dpcontext></dpcontext> so it becomes more Drupal-ish!
        $excerpt = preg_replace('|\</?dpcontext\>|', '', $para);

        // prevent really long link text
        $limit = 200;
        if (drupal_strlen($context[1]) > $limit) {
          $context[1] = drupal_substr($context[1], 0, $limit) . ' …';
        }

        $marker = '<dpcontext>' . $context[1] . '</dpcontext>'; // set up our marker
        $excerpt = str_replace($context[0], $marker, $excerpt); // swap out the link for our marker
        $excerpt = strip_tags($excerpt, '<dpcontext>'); // strip all tags but our context marker
        $excerpt = trim($excerpt);
        $preg_marker = preg_quote($marker);
        $excerpt = preg_replace("|.*?\s(.{0,$limit}$preg_marker.{0,$limit})\s.*|s", '$1', $excerpt);
        $excerpt = strip_tags($excerpt); // YES, again, to remove the marker wrapper

        break;
      }
    }
    return array($title, $excerpt);
  }

  /**
   * The default label for a ref-back is the page-title of the referring URL
   * (or the URL if the title is not available).
   */
  protected function defaultLabel() {
    return $this->title ?: $this->url;
  }

  /**
   * Ref-back entities have no inherent URL.
   */
  public function defaultUri() {
    return NULL;
  }

  /**
   * Validation handler.
   */
  public function validate() {
    // Typical validation:
    // - Node is valid and permits ref-backs
    // - Remote article contains a link to the source node/URL
    // - Remote article (title/excerpt/general content) isn't spammy

    // Ensure that we have title and excerpt to carry on with validation
    if (empty($this->title) || empty($this->excerpt)) {
      // Populate title and excerpt if possible.
      if (   $data =self::vinculumGetExcerpt(  $this->url, url( "node/{$this->nid}", array('absolute' =>      TRUE) )  )   ) {
        list($title, $excerpt) = $data;
        $this->title   = $this->title ?: $title;
        $this->excerpt = $this->excerpt ?: $excerpt;
      }
      else {
        // If the title couldn't be extracted programmatically, set the title to
        // match the URL.
        $this->title = $this->url;
      }
    }

    // The 3 required parameters must be provided, but this does not guarantee
    // that the node exists.
    if (empty($this->handler)) {
      $this->last_error_code = 0;
      $this->last_error_message = 'The refback-handler must be provided.';
      return FALSE;
    }
    elseif (empty($this->nid) && isset($this->nidSource) && $this->nidSource == 'url') {
      $this->last_error_code = VINCULUM_ERROR_LOCAL_NODE_NOT_FOUND;
      $this->last_error_message = 'Target for ref-back not found.';
      return FALSE;
    }
    elseif (empty($this->nid)) {
      $this->last_error_code = 0;
      $this->last_error_message = 'A node ID or URL must be provided.';
      return FALSE;
    }


    // All validation handlers are expected to provide an array of error-code
    // and error-message to recognise an error.  Only the first error is
    // reported.
    foreach (module_implements('vinculum_received_validate') as $module) {
      $result = module_invoke($module, 'vinculum_received_validate', $this);
      if (is_array($result)) {
        list($error_code, $error_message) = $result;
        $this->last_error_code = $error_code;
        $this->last_error_message = $error_message;
        return FALSE;
      }
    }
    return TRUE;
  }

  /**
   * Save a received ref-back.
   *
   * @return Int
   * - SAVED_NEW     If a new ref-back was saved.
   * - SAVED_UPDATED If an existing ref-back was updated.
   * - FALSE         If the request didn't validate.
   */
  public function save() {
    // Invoke validation.
    if ($this->validate()) {
      // Save the entity.
      // Entity presave/update/insert hooks will be invoked by the entity API
      // controller.
      // @see hook_vinculum_received_presave()
      // @see hook_vinculum_received_insert()
      // @see hook_vinculum_received_update()
      return parent::save();
    }
    return FALSE;
  }



}
