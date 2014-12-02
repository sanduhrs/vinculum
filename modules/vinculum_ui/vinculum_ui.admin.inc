<?php
/**
 * @file
 * Admin pages for the vinculum module.
 */

/**
 * Menu callback for admin/config/content/vinculum.
 *
 * Allows the handlers to be prioritised.
 */
function vinculum_ui__settings_form($form) {

  $form['vinculum_validate_remote_pages'] = array(
    '#type' => 'checkbox',
    '#title' => t('Validate remote pages'),
    '#description' => t('When linkbacks are received, check that the remote URL is reachable and contains a link to the referenced node.'),
    '#default_value' => variable_get('vinculum_validate_remote_pages', FALSE),
  );

  // Fetch a list of the available handlers (and reset the  handler-cache to
  // ensure that this page always lists the current handler-set).
  //
  // Handlers are listed by module-weight.
  $handlers = vinculum_load_all_handlers(TRUE);

  $form['handlers'] = array(
    '#tree' => TRUE,
    '#theme' => 'vinculum_ui__settings_form__handlers_element',
    '#handlers' => $handlers,
  );
  foreach ($handlers as $handler) {
    $form['handlers'][$handler->module] = array(
      '#type' => 'weight',
      '#title' => check_plain($handler->protocol),
      '#default_value' => $handler->weight,
      '#attributes' => array(
        'class' => array('vinculum-handler-weight'),
      ),
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Submit handler for the configuration form.
 */
function vinculum_ui__settings_form_submit(&$form, &$form_state) {
  $weights = $form_state['values']['handlers'];
  vinculum_set_weights($weights);
  variable_set('vinculum_validate_remote_pages', (bool) $form_state['values']['vinculum_validate_remote_pages']);

  drupal_set_message(t('Your settings have been saved.'));
}

/**
 * Theme handler for the list of handlers on the vinculum configuration form.
 *
 * This transforms the list of handlers into a draggable table.
 */
function theme_vinculum_ui__settings_form__handlers_element($variables) {
  $form = $variables['form'];

  // Form the handlers into a table.
  $header = array(
    t('Protocol'),
    t('Weight'),
  );
  $rows = array();
  foreach ($form['#handlers'] as $key => $handler) {
    // Remove the element's title attribute (it's in the table header).
    $form[$key]['#title'] = '';
    $rows[] = array(
      'data' => array(
        'protocol' => $form['#handlers'][$key]->protocol,
        'weight' => drupal_render($form[$key]),
      ),
      'class' => array('draggable'),
    );
  }

  $output = '';
  $output .= theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => 'vinculum-table'),
    'empty' => t('There are no handlers available. Enable one or more vinculum handler modules.'),
  ));
  $output .= drupal_render_children($form);
  drupal_add_tabledrag('vinculum-table', 'order', 'sibling', 'vinculum-handler-weight');

  return $output;
}

/**
 * Report of all ref-backs sent.
 */
function vinculum_ui__report_sent() {
  $header = array(
    array('data' => t('Date'),       'field' => 'timestamp',  'sort' => 'desc'),
    array('data' => t('From'),       'field' => 'n.title'),
    array('data' => t('Remote URL'), 'field' => 'url'),
    array('data' => t('Handler'),    'field' => 'handler'),
  );

  $query = db_select('node_vinculum_sent', 'l');
  $query->join('node', 'n', 'n.nid = l.nid');
  $query->extend('PagerDefault')
    ->limit(1)
    ->extend('TableSort')
    ->orderByHeader($header)
    ->fields('n', array('title'))
    ->fields('l');
  $results = $query->execute();

  $handlers = vinculum_get_handler();

  $rows = array();
  foreach ($results as $result) {
    $rows[] = array(
      date('d/m/Y H:i', $result->timestamp),
      l($result->title, "node/{$result->nid}"),
      l($result->url, $result->url),
      isset($handlers[$result->handler]) ? $handlers[$result->handler]->protocol : $result->handler,
    );
  }

  $html = '';
  $html .= theme('table', array('header' => $header, 'rows' => $rows));
  $html .= theme('pager', array('tags' => array()));
  return $html;
}

/**
 * Report of all ref-backs received.
 */
function vinculum_ui__report_received() {
  $header = array(
    array(
      'data' => t('Date'), 'field' => 'timestamp', 'sort' => 'desc'),
    array('data' => t('Content'),    'field' => 'n.title'),
    array('data' => t('Remote URL'), 'field' => 'url'),
    array('data' => t('Handler'),    'field' => 'handler'),
  );

  $query = db_select('node_vinculum_received', 'l');
  $query->join('node', 'n', 'n.nid = l.nid');
  $query->extend('PagerDefault')
    ->limit(1)
    ->extend('TableSort')
    ->orderByHeader($header)
    ->fields('n', array('title'))
    ->fields('l');

  $results = $query->execute();

  $handlers = vinculum_get_handler();

  $rows = array();
  foreach ($results as $result) {
    $rows[] = array(
      date('d/m/Y H:i', $result->timestamp),
      l($result->title, "node/{$result->nid}"),
      l($result->url, $result->url),
      isset($handlers[$result->handler]) ? $handlers[$result->handler]->protocol : $result->handler,
    );
  }

  $html = '';
  $html .= theme('table', array('header' => $header, 'rows' => $rows));
  $html .= theme('pager', array('tags' => array()));
  return $html;
}
