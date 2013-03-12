<?php

/**
 * Generic Views field handler for showing links to operations.
 *
 * Based on views_handler_field_node_link.
 */
class entity_operations_handler_field_operation_link extends views_handler_field_entity {

  /**
   * Define defaults for options.
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['text'] = array('default' => '', 'translatable' => TRUE);
    return $options;
  }

  /**
   * Build the options form.
   */
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

  /**
   * Run before any fields are rendered.
   */
  function pre_render(&$values) {
    // This gets us the entities that correspond to our rows.
    parent::pre_render($values);

    // Get the handler for the operation.
    $this->operation_handler = entity_operations_get_entity_operation_handler($this->entity_type, $this->definition['operation path']);

    // We handle access here, because this lets us determine overall access
    // to all of the rows, and lets us remove ourselves from the view if the
    // user has access to nothing.
    // We can't use access(), which is the method we're meant to use to deny
    // access to a field wholesale, because that is called before we have any
    // results (and thus any entities) from the view.
    // @see http://drupal.org/node/1915230
    $this->row_access = array();
    // We have to check the entities array, as
    // views_handler_field_entity::pre_render() only sets it if there is a
    // result for the view.
    if (!empty($this->entities)) {
      foreach ($this->entities as $entity) {
        // Build up an array of access for each row for render() to use.
        $id = $entity->identifier();
        $this->row_access[$id] = $this->operation_handler->access($this->entity_type, $entity);
      }
    }

    // The overall access to this field is whether the user has access to any
    // of the rows.
    $overall_access = count(array_filter($this->row_access));

    // If the user has access to this operation on none of the rows, we assume
    // they generally lack the right permissions, and remove ourselves wholesale
    // from this view.
    // This ensures, for instance, that the user doesn't get an empty (and
    // likely meaningless!) column in a table view.
    if (!$overall_access) {
      unset($this->view->field[$this->field]);
      // Just bail now, no point.
      return;
    }

    // Determine whether we should use a destination on the link.
    $operation_info = $this->operation_handler->operationInfo();
    if (!empty($operation_info['uses form'])) {
      $this->options['alter']['query'] = drupal_get_destination();
    }
  }

  /**
   * Render the field.
   */
  function render($values) {
    if ($entity = $this->get_value($values)) {
      $id = $entity->identifier();
      // Check the access we got in pre_render().
      if (!$this->row_access[$id]) {
        return;
      }

      return $this->render_link($entity, $values);
    }
  }

  /**
   * Render this field as a link.
   */
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
