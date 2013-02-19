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

/**
 * Implements hook_views_data_alter().
 *
 * Add Views fields for links to entity operations that support it.
 */
function entity_operations_views_data_alter(&$data) {
  $entities_operations_info = entity_operations_get_operation_info();
  foreach ($entities_operations_info as $entity_type => $entity_operations_info) {
    $entity_info = entity_get_info($entity_type);

    // Only add operations to entities if their base table has Views data.
    $base_table = $entity_info['base table'];
    if (isset($data[$base_table])) {
      foreach ($entity_operations_info['operations'] as $operation_name => $operation) {
        // Get the handler and the operation info.
        $operation_handler = entity_operations_get_entity_operation_handler($entity_type, $operation_name);
        $operation_info = $operation_handler->operationInfo();

        if (empty($operation_info['provides views field'])) {
          // Skip operations that don't declare that they provide a views field.
          continue;
        }

        $data[$base_table]['entity_operation_' . $operation_name] = array(
          // TODO: better title.
          'title' => t('Entity operation: @op', array('@op' => $operation_name)),
          // TODO: better description.
          'help' => t('Entity operation: @op', array('@op' => $operation_name)),
          'field' => array(
            'real field' => $entity_info['entity keys']['id'],
            'handler' => 'entity_operations_handler_field_operation_link',
            // Data for our handler to find.
            'operation path' => $operation_name,
          ),
        );
      }
    }
  }
}
