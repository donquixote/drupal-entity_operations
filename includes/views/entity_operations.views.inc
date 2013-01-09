<?php
/**
 * @file entity_operations.views.inc
 * Contains Views hooks.
 */

/**
 * Implements hook_views_plugins().
 */
function entity_operations_views_plugins() {
  return array(
    'display' => array(
      'entity_operation' => array(
        'title' => t('Entity tab'),
        'help' => t("Display the view astab on an entity."),
        'handler' => 'entity_operations_plugin_display_operation_tab',
        'theme' => 'views_view',
        'provides entity operation' => TRUE,
        'use ajax' => TRUE,
        'use pager' => TRUE,
        'use more' => TRUE,
        'accept attachments' => TRUE,
        'admin' => t('Entity tab'),
        'help topic' => 'display-entity',
      ),
    ),
  );
}
