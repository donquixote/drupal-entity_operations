<?php

/**
 * Generic Views field handler for showing links to operations.
 *
 * Based on views_handler_field_node_link.
 */
class entity_operations_handler_field_operation_link extends views_handler_field_entity {

  function option_definition() {
    $options = parent::option_definition();
    $options['text'] = array('default' => '', 'translatable' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
    );
    parent::options_form($form, $form_state);

    // The path is set by render_link function so don't allow to set it.
    $form['alter']['path'] = array('#access' => FALSE);
    $form['alter']['external'] = array('#access' => FALSE);
  }

  function pre_render(&$values) {
    parent::pre_render(&$values);

    // Get the handler for the operation.
    $this->operation_handler = entity_operations_get_entity_operation_handler($this->entity_type, $this->definition['operation path']);

    // Determine whether we should use a destination on the link.
    $operation_info = $this->operation_handler->operationInfo();
    if (!empty($operation_info['uses form'])) {
      $this->options['alter']['query'] = drupal_get_destination();
    }
  }

  function render($values) {
    if ($entity = $this->get_value($values)) {
      // Check access for the operation.
      if ($this->operation_handler->operationAccess($this->entity_type, $entity) === FALSE) {
        return;
      }
      if ($this->operation_handler->userAccess($this->entity_type, $entity) === FALSE) {
        return;
      }
      // TODO: check access verb!

      return $this->render_link($entity, $values);
    }
  }

  function render_link($entity, $values) {
    // Get the entity URI and append the operation path component.
    $uri = entity_uri($this->entity_type, $entity);
    $uri['path'] .= '/' . $this->definition['operation path'];

    $this->options['alter']['make_link'] = TRUE;
    $this->options['alter']['path'] = $uri['path'];
    // The 'query' value is set in pre_render() as it's common to the whole
    // View result.

    $text = !empty($this->options['text']) ? $this->options['text'] : $this->definition['operation path'];
    return $text;
  }

}