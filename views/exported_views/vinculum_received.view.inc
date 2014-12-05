<?php

$view = new view;
$view->name = 'vinculum_received';
$view->description = 'List the ref-backs received';
$view->tag = 'default';
$view->base_table = 'node_vinculum_received';
$view->human_name = 'Ref-backs received';
$view->core = 7;
$view->api_version = '3.0';
$view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

/* Display: Master */
$handler = $view->new_display('default', 'Master', 'default');
$handler->display->display_options['title'] = 'Related links';
$handler->display->display_options['access']['type'] = 'none';
$handler->display->display_options['cache']['type'] = 'none';
$handler->display->display_options['query']['type'] = 'views_query';
$handler->display->display_options['query']['options']['query_comment'] = FALSE;
$handler->display->display_options['exposed_form']['type'] = 'basic';
$handler->display->display_options['pager']['type'] = 'some';
$handler->display->display_options['pager']['options']['items_per_page'] = '10';
$handler->display->display_options['style_plugin'] = 'default';
$handler->display->display_options['row_plugin'] = 'fields';
/* Relationship: Ref-back: Node nid */
$handler->display->display_options['relationships']['node']['id'] = 'node';
$handler->display->display_options['relationships']['node']['table'] = 'node_vinculum_received';
$handler->display->display_options['relationships']['node']['field'] = 'node';
$handler->display->display_options['relationships']['node']['label'] = 'Node';
$handler->display->display_options['relationships']['node']['required'] = 1;
/* Field: Ref-back: Title */
$handler->display->display_options['fields']['title']['id'] = 'title';
$handler->display->display_options['fields']['title']['table'] = 'node_vinculum_received';
$handler->display->display_options['fields']['title']['field'] = 'title';
$handler->display->display_options['fields']['title']['label'] = '';
$handler->display->display_options['fields']['title']['alter']['alter_text'] = 0;
$handler->display->display_options['fields']['title']['alter']['make_link'] = 0;
$handler->display->display_options['fields']['title']['alter']['absolute'] = 0;
$handler->display->display_options['fields']['title']['alter']['external'] = 0;
$handler->display->display_options['fields']['title']['alter']['replace_spaces'] = 0;
$handler->display->display_options['fields']['title']['alter']['trim_whitespace'] = 0;
$handler->display->display_options['fields']['title']['alter']['nl2br'] = 0;
$handler->display->display_options['fields']['title']['alter']['word_boundary'] = 1;
$handler->display->display_options['fields']['title']['alter']['ellipsis'] = 1;
$handler->display->display_options['fields']['title']['alter']['more_link'] = 0;
$handler->display->display_options['fields']['title']['alter']['strip_tags'] = 0;
$handler->display->display_options['fields']['title']['alter']['trim'] = 0;
$handler->display->display_options['fields']['title']['alter']['html'] = 0;
$handler->display->display_options['fields']['title']['element_label_colon'] = FALSE;
$handler->display->display_options['fields']['title']['element_default_classes'] = 1;
$handler->display->display_options['fields']['title']['hide_empty'] = 0;
$handler->display->display_options['fields']['title']['empty_zero'] = 0;
$handler->display->display_options['fields']['title']['hide_alter_empty'] = 1;
$handler->display->display_options['fields']['title']['link_to_refback'] = 1;
/* Field: Ref-back: Date received */
$handler->display->display_options['fields']['created']['id'] = 'created';
$handler->display->display_options['fields']['created']['table'] = 'node_vinculum_received';
$handler->display->display_options['fields']['created']['field'] = 'created';
$handler->display->display_options['fields']['created']['label'] = '';
$handler->display->display_options['fields']['created']['alter']['alter_text'] = 0;
$handler->display->display_options['fields']['created']['alter']['make_link'] = 0;
$handler->display->display_options['fields']['created']['alter']['absolute'] = 0;
$handler->display->display_options['fields']['created']['alter']['external'] = 0;
$handler->display->display_options['fields']['created']['alter']['replace_spaces'] = 0;
$handler->display->display_options['fields']['created']['alter']['trim_whitespace'] = 0;
$handler->display->display_options['fields']['created']['alter']['nl2br'] = 0;
$handler->display->display_options['fields']['created']['alter']['word_boundary'] = 1;
$handler->display->display_options['fields']['created']['alter']['ellipsis'] = 1;
$handler->display->display_options['fields']['created']['alter']['more_link'] = 0;
$handler->display->display_options['fields']['created']['alter']['strip_tags'] = 0;
$handler->display->display_options['fields']['created']['alter']['trim'] = 0;
$handler->display->display_options['fields']['created']['alter']['html'] = 0;
$handler->display->display_options['fields']['created']['element_label_colon'] = FALSE;
$handler->display->display_options['fields']['created']['element_default_classes'] = 1;
$handler->display->display_options['fields']['created']['hide_empty'] = 0;
$handler->display->display_options['fields']['created']['empty_zero'] = 0;
$handler->display->display_options['fields']['created']['hide_alter_empty'] = 1;
$handler->display->display_options['fields']['created']['date_format'] = 'time ago';
/* Field: Ref-back: Excerpt */
$handler->display->display_options['fields']['excerpt']['id'] = 'excerpt';
$handler->display->display_options['fields']['excerpt']['table'] = 'node_vinculum_received';
$handler->display->display_options['fields']['excerpt']['field'] = 'excerpt';
$handler->display->display_options['fields']['excerpt']['label'] = '';
$handler->display->display_options['fields']['excerpt']['alter']['alter_text'] = 0;
$handler->display->display_options['fields']['excerpt']['alter']['make_link'] = 0;
$handler->display->display_options['fields']['excerpt']['alter']['absolute'] = 0;
$handler->display->display_options['fields']['excerpt']['alter']['external'] = 0;
$handler->display->display_options['fields']['excerpt']['alter']['replace_spaces'] = 0;
$handler->display->display_options['fields']['excerpt']['alter']['trim_whitespace'] = 0;
$handler->display->display_options['fields']['excerpt']['alter']['nl2br'] = 0;
$handler->display->display_options['fields']['excerpt']['alter']['word_boundary'] = 1;
$handler->display->display_options['fields']['excerpt']['alter']['ellipsis'] = 1;
$handler->display->display_options['fields']['excerpt']['alter']['more_link'] = 0;
$handler->display->display_options['fields']['excerpt']['alter']['strip_tags'] = 0;
$handler->display->display_options['fields']['excerpt']['alter']['trim'] = 0;
$handler->display->display_options['fields']['excerpt']['alter']['html'] = 0;
$handler->display->display_options['fields']['excerpt']['element_label_colon'] = FALSE;
$handler->display->display_options['fields']['excerpt']['element_default_classes'] = 1;
$handler->display->display_options['fields']['excerpt']['hide_empty'] = 0;
$handler->display->display_options['fields']['excerpt']['empty_zero'] = 0;
$handler->display->display_options['fields']['excerpt']['hide_alter_empty'] = 1;
$handler->display->display_options['fields']['excerpt']['link_to_refback'] = 0;
/* Sort criterion: Ref-back: Date received */
$handler->display->display_options['sorts']['created']['id'] = 'created';
$handler->display->display_options['sorts']['created']['table'] = 'node_vinculum_received';
$handler->display->display_options['sorts']['created']['field'] = 'created';
$handler->display->display_options['sorts']['created']['order'] = 'DESC';

/* Display: Block */
$handler = $view->new_display('block', 'Block', 'block');
$handler->display->display_options['defaults']['arguments'] = FALSE;
/* Contextual filter: Ref-back: Node nid */
$handler->display->display_options['arguments']['node']['id'] = 'node';
$handler->display->display_options['arguments']['node']['table'] = 'node_vinculum_received';
$handler->display->display_options['arguments']['node']['field'] = 'node';
$handler->display->display_options['arguments']['node']['default_action'] = 'default';
$handler->display->display_options['arguments']['node']['default_argument_type'] = 'node';
$handler->display->display_options['arguments']['node']['default_argument_skip_url'] = 0;
$handler->display->display_options['arguments']['node']['summary']['number_of_records'] = '0';
$handler->display->display_options['arguments']['node']['summary']['format'] = 'default_summary';
$handler->display->display_options['arguments']['node']['summary_options']['items_per_page'] = '25';
$handler->display->display_options['arguments']['node']['break_phrase'] = 0;
$handler->display->display_options['arguments']['node']['not'] = 0;
