<?php
/**
 * @file
 * Contains the vinculum_views_handler_field_refback class.
 */

/**
 * A handler to link ref-back fields to the originating referrer.
 *
 * @ingroup views_field_handlers
 */
class vinculum_views_handler_field_refback_text extends views_handler_field {

  function init(&$view, &$options) {
    parent::init($view, $options);
    if (!empty($this->options['link_to_refback'])) {
      $this->additional_fields['url'] = array('table' => 'node_vinculum_received', 'field' => 'url');
    }
  }

  /**
   * Provide options for this handler.
   */
  public function option_definition() {
    $options = parent::option_definition();
    $options['link_to_refback'] = array('default' => TRUE);
    return $options;
  }

  /** 
   * Provide link to node option
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_refback'] = array(
      '#title' => t('Link this field to its referrer URL'),
      '#description' => t('This will override any other link you have set.'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_refback']),
    );
  }

  /**
   * Render whatever the data is as a link to the node.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($data, $values) {
    if (!empty($this->options['link_to_refback'])) {
      if ($data !== NULL && $data !== '') {
        $this->options['alter']['make_link'] = TRUE;
        $this->options['alter']['path'] = $values->{$this->aliases['url']};
      }
      else {
        $this->options['alter']['make_link'] = FALSE;
      }
    }
    return $data;
  }

  function render($values) {
    return $this->render_link(check_plain($values->{$this->field_alias}), $values);
  }
}
