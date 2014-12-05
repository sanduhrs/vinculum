<?php
/**
 * @file
 * Describe the VinculumReceived entity meta-data.
 */

/**
 * Implements hook_entity_property_info().
 */
function vinculum_entity_property_info() {
  // drupal_set_message('hook_entity_property_info fired');
  $info = array();

  // Add meta-data about the basic node properties.
  $properties = &$info['vinculum_received']['properties'];

  // Properties:
  // - Node NID
  // - URL of the article which linked to us
  // - Title   (extracted from the <title> tag)
  // - Excerpt (extracted from the body content)
  // - IP address
  // - Timestamp

  $properties['node'] = array(
    'label' => t("Node"),
    'type' => 'node',
    'description' => t("The node referenced in a ref-back."),
    'schema field' => 'nid',
    'required' => TRUE,
  );
  $properties['url'] = array(
    'label' => t("URL"),
    'type' => 'uri',
    'description' => t("The URL of the remote article."),
    'schema field' => 'url',
    'required' => TRUE,
  );
  $properties['title'] = array(
    'label' => t("Title"),
    'type' => 'text',
    'description' => t("The title of the remote article."),
    'schema field' => 'title',
  );
  $properties['excerpt'] = array(
    'label' => t("Excerpt"),
    'type' => 'text',
    'description' => t("An excerpt of the remote article."),
    'schema field' => 'excerpt',
  );
  $properties['ip_address'] = array(
    'label' => t("IP Address"),
    'type' => 'text',
    'description' => t("The IP address of the computer the ref-back was posted from."),
    'schema field' => 'origin_ip',
  );
  $properties['created'] = array(
    'label' => t("Date received"),
    'type' => 'date',
    'description' => t("The date the ref-back was received."),
    'schema field' => 'created',
  );
  $properties['handler'] = array(
    'label' => t("Ref-back module"),
    'type' => 'text',
    'description' => t("The module which recorded the ref-back."),
    'schema field' => 'handler',
  );

  return $info;
}
