<?php

/**
 * @file entity_operations_test.module
 * Contains form builders for entity forms.
 */

/**
 * The test entity form.
 */
function entity_operations_test_form($form, &$form_state, $entity) {
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#description' => t('The title of this entity.'),
    '#default_value' => isset($entity->title) ? $entity->title : '',
    '#required' => TRUE,
  );

  field_attach_form('entity_operations_test', $entity, $form, $form_state);

  $form['actions'] = array(
    '#weight' => 10,
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Validate handler for edit/add form.
 */
function entity_operations_test_form_validate($form, &$form_state) {
  // Notify field widgets to validate their data.
  entity_form_field_validate('entity_operations_test', $form, $form_state);
}

/**
 * Submit handler for edit/add form.
 */
function entity_operations_test_form_submit($form, &$form_state) {
  $entity = entity_ui_form_submit_build_entity($form, $form_state);

  $entity->uid = $GLOBAL['user']->uid;

  // Save the entity.
  $entity->save();

  // Redirect to the entity.
  $uri = $entity->uri();

  $form_state['redirect'] = $uri['path'];
}
