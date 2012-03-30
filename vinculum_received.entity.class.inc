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
  public $timestamp;
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
   * - timestamp       Defaults to time()
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
      'timestamp'  => REQUEST_TIME,
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
  public static function lookupNid($url) {
    // Strip the domain from the URL (if present).
    if (preg_match('#^(https?://[^/]+/).*#', $url, $matches)) {
      $url = drupal_substr($url, drupal_strlen($matches[1]));
    }

    // Lookup URL aliases.
    if ($local_path = drupal_lookup_path('source', $url)) {
      $url = $local_path;
    }

    if (preg_match('#^node/(\d+)$#', $url, $matches)) {
      return (int) $matches[1];
    }
    return FALSE;
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
      return TRUE;
    }
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
