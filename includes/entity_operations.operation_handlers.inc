<?php

/**
 * @file
 * Contains handler classes for basic operations as well as base classes:
 *
 * - EntityOperationsOperationBase: abstract base class for all operations.
 *  - EntityOperationsOperationPage: parent class for operations that provide
 *    a page.
 *    - EntityOperationsOperationEntityView: displays the entity.
 *      - EntityOperationsOperationEntityViewOperations: displays the entity,
 *        with available operations in a fieldset.
 *    - EntityOperationsOperationDevel: devel output for the entity.
 *  - EntityOperationsOperationForm: parent class for form operations.
 *    - EntityOperationsOperationEdit: the edit form for the entity.
 *    - EntityOperationsOperationAction: parent class for actions on the entity.
 *      - EntityOperationsOperationPublish: basic publish action.
 *      - EntityOperationsOperationUnPublish: basic unpublish action.
 *  - EntityOperationsOperationAdd: the add form for the entity.
 */

/**
 * Abstract base class for operation handlers.
 */
abstract class EntityOperationsOperationBase {

  protected $entityType;

  /**
   * The entity info.
   */
  protected $entityInfo;

  /**
   * The operations controller. Only set when building hook_menu().
   */
  protected $controller;

  /**
   * Returns basic information about the operation.
   *
   * @return
   *  An array with the following keys:
   *  - 'provides tab': Whether this operation should provide a tab on the
   *    entity.
   *  - 'uses form': If 'provides tab' is TRUE, whether this operation's tab
   *    is a form rather than a page.
   *  - 'provides vbo': Whether this operation should be exposed to Views Bulk
   *    Operations.
   */
  function operationInfo() {
    return array(
      // Default behaviour is to provide a tab.
      'provides tab' => TRUE,
      'uses form' => FALSE,
      'provides vbo' => FALSE,
    );
  }

  /**
   * Only needs the controller when used by the controller.
   * TODO!!! URRRGH!!! UGLY PATTERN!
   */
  public function __construct($entity_type, $ui_controller = NULL) {
    $this->entityType = $entity_type;
    $this->controller = $ui_controller;
  }

  /**
   * Properties for the menu item specific to this handler.
   */
  function menu_item($operation_path, $operation_definition) {
    return array();
  }

  /**
   * Determine whether the action may be performed on the given entity.
   *
   * This should take care of determining whether the operation itself makes
   * sense, irrespective of the user performing it.
   *
   * For example, a 'publish' operation should deny access here if the entity
   * is already published.
   *
   * @return
   *  FALSE is the operation should not be allowed, NULL if it should be allowed
   *  from a logical standpoint and further access control passed on to the
   *  user permissions.
   */
  function operationAccess($entity_type, $entity) {
  }

  /**
   * Determine access to the operation for the user.
   *
   * @param $entity_type
   *  The entity type.
   * @param $entity
   *  The current entity.
   * @param $account
   *  (Optional) The account to check access for. Defaults to the current user.
   *
   * @return
   *  FALSE is the operation should not be allowed, TRUE if access should be
   *  granted and NULL if further access control should be passed on to the
   *  entity access system.
   */
  function userAccess($entity_type, $entity, $account = NULL) {
  }

  /**
   * Execute the operation.
   */
  function execute($entity_type, $entity) {
  }

}

/**
 * Default handler for a page callback.
 */
class EntityOperationsOperationPage extends EntityOperationsOperationBase {

  /**
   * The operation for entity_access().
   */
  public $access_verb = 'view';

  /**
   * Returns the menu item for this operation.
   */
  function menu_item($operation_path, $operation_definition) {
    $item = array(
      // This takes care of calling our page() method, and is set here as a
      // convenience.
      // Subclasses however may specify any page callback they wish.
      'page callback' => 'entity_operations_operation_page_callback',
      'page arguments' => array(
        $this->entityType,
        // We need to pass $this to get_class() because we want the class of
        // the current object, not the class we are sitting in.
        get_class($this),
        // TODO!!! UGLY!!
        $this->controller->loader_position,
      ),
      'access callback' => 'entity_operations_operation_access_callback',
      'access arguments' => array($this->entityType, get_class($this), $this->controller->loader_position),
    );

    // Add in what the parent class has.
    $item += parent::menu_item($operation_path, $operation_definition);
    return $item;
  }

  /**
   * Page callback.
   */
  function page($entity_type, $entity) {
    return 'This is my operation page.';
  }

}

/**
 * Base operation for forms.
 */
class EntityOperationsOperationForm extends EntityOperationsOperationBase {

  public $access_verb = NULL;

  /**
   * Returns basic information about the operation.
   */
  function operationInfo() {
    return array(
      'provides tab' => TRUE,
      'uses form' => TRUE,
      'provides vbo' => FALSE,
    );
  }

  /**
   * Properties for the menu item specific to this handler.
   */
  function menu_item($operation_path, $operation_definition) {
    return array(
      'page callback' => 'drupal_get_form',
      'page arguments' => array(
        'entity_operations_operation_form',
        $this->entityType,
        get_class($this),
        $operation_path,
        $this->controller->loader_position, // Provides the entity as a parameter.
      ),
      'access callback' => 'entity_operations_operation_access_callback',
      'access arguments' => array($this->entityType, get_class($this), $this->controller->loader_position),
    );
  }

  /**
   * Helper for simple confirmation forms.
   *
   * Based on confirm_form(), which we can't use because it uses the page
   * title to show the question, whereas we output in a tab.
   */
  function confirmForm($form, $entity_type, $entity, $question, $description = NULL, $yes = NULL, $no = NULL) {
    $description = isset($description) ? $description : t('This action cannot be undone.');

    // Prepare cancel link.
    if (isset($_GET['destination'])) {
      $options = drupal_parse_url(urldecode($_GET['destination']));
    }
    else {
      $options = $entity->uri();
    }

    $form['#attributes']['class'][] = 'confirmation';
    $form['question'] = array(
      '#prefix' => '<p>',
      '#markup' => $question,
      '#suffix' => '</p>',
    );
    $form['description'] = array(
      '#prefix' => '<p>',
      '#markup' => $description,
      '#suffix' => '</p>',
    );

    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => $yes ? $yes : t('Confirm'),
    );
    $form['actions']['cancel'] = array(
      '#type' => 'link',
      '#title' => $no ? $no : t('Cancel'),
      '#href' => $options['path'],
      '#options' => $options,
    );
    // By default, render the form using theme_confirm_form().
    if (!isset($form['#theme'])) {
      $form['#theme'] = 'confirm_form';
    }
    return $form;
  }

  /**
   * Form builder for this operation.
   *
   * To get different parameters, override menu_item().
   *
   * @param $entity_type
   *  The entity type.
   * @param $entity
   *  The current entity.
   * @param $operation_path
   *  The operation path of the current operation. This is the key of the
   *  definition array in hook_entity_info().
   */
  function form($form, &$form_state, $entity_type, $entity, $operation_path) {
  }

  /**
   * Form validation handler for this operation.
   *
   * Receives the same parameters as the form builder as a convenience.
   */
  function formValidate($form, &$form_state, $entity_type, $entity, $operation_path) {
  }

  /**
   * Form submit handler for this operation.
   *
   * Receives the same parameters as the form builder as a convenience.
   */
  function formSubmit($form, &$form_state, $entity_type, $entity, $operation_path) {
  }

  /**
   * Helper function to set the form redirect to the entity URI.
   */
  function redirect_entity_uri($entity_type, $entity) {
    $uri = entity_uri($entity_type, $entity);
    return $uri['path'];
  }

  /**
   * ARRRGH needs tons of data we don't have here!
   */
  function redirect_default_operation($entity_type, $entity) {
    $entity_info = entity_get_info($entity_type);
    // ARGHL need to delve way too deep into stuff!
  }
}

/**
 * Op handler for viewing an entity.
 */
class EntityOperationsOperationEntityView extends EntityOperationsOperationPage {

  public $access_verb = 'view';

  function page($entity_type, $entity) {
    //dsm($entity, 'handler page');
    drupal_set_title($entity->label());
    // Pass in EntityAPI's defaults for the first two parameters so we can
    // set $page to TRUE.
    return $entity->view('full', NULL, TRUE);
  }

}

/**
 * Extended op handler for viewing an entity with operation buttons shown.
 *
 * This adds operation buttons for those with 'entity view' set to TRUE.
 *
 * @see entity_operations_get_entity_operations_fieldset().
 */
class EntityOperationsOperationEntityViewOperations extends EntityOperationsOperationEntityView {

  public $access_verb = 'view';

  function page($entity_type, $entity) {
    $build = parent::page($entity_type, $entity);

    // Add the automatic entity view operations to the build array.
    $build['operations'] = entity_operations_get_entity_operations_fieldset($entity);
    $build['operations']['#weight'] = 10;

    return $build;
  }

}

/**
 * Op handler for viewing devel output for an entity.
 */
class EntityOperationsOperationDevel extends EntityOperationsOperationPage {

  public $access_verb = 'view';

  /**
   * Determine whether the action may be performed on the given entity.
   */
  function operationAccess($entity_type, $entity) {
    // This operation depends on devel module.
    return module_exists('devel');
  }

  /**
   * Determine whether the user has access to this operation.
   */
  function userAccess($entity_type, $entity, $account = NULL) {
    return user_access('access devel information');
  }

  /**
   * Page callback.
   */
  function page($entity_type, $entity) {
    drupal_set_title($entity->label());
    $output = kprint_r($entity, TRUE);
    return $output;
  }

}

/**
 * Op handler for editing an entity: our form.
 THIS IS PROBABLY YAGNI!
 */
/*
class EntityOperationsOperationEdit extends EntityOperationsOperationForm {

  public $access_verb = 'edit';
  // Title: "Edit %entity".

  function form($form, &$form_state, $entity_type, $entity) {
    drupal_set_title(t('Edit %entity', array(
      '%entity' => $entity->label(),
    )), PASS_THROUGH);

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => 'Submit!',
    );

    return $form;
  }

  function formSubmit($form, &$form_state, $entity_type, $entity) {
    dsm($entity);
    $form_state['redirect'] = $this->redirect_entity_uri($entity_type, $entity);
  }

}
*/

/**
 * Op handler for adding an entity: EntityAPI form.
 *
 * Add is a bit of a special case, as it doesn't use an entity loader.
 *
 * @TODO: support bundles. EEEP!
 *
 * @TODO: add a common abstract parent class with the Edit op, to avoid
 * repetition in menu_item().
 */
class EntityOperationsOperationAdd extends EntityOperationsOperationBase {

  public $access_verb = 'create';
  // Title: "Edit %entity".

  /**
   * Returns basic information about the operation.
   */
  function operationInfo() {
    return array(
      'provides tab' => FALSE,
      'uses form' => TRUE,
      'provides vbo' => FALSE,
    );
  }

  /**
   * Properties for the menu item specific to this handler.
   */
  function menu_item($operation_path, $operation_definition) {
    $item = array(
      // Special path which doesn't have an entity loader.
      'path' => $this->controller->path . '/add',
      // We don't want a tab.
      'type' => MENU_CALLBACK,
      'page callback' => 'entity_ui_get_form',
      'page arguments' => array(
        $this->entityType,
        NULL,
        'add',
      ),
      'access callback' => 'entity_operations_operation_access_callback',
      'access arguments' => array(
        $this->entityType,
        get_class($this),
        $this->controller->loader_position,
      ),
    );

    // Use the file setting from entity info 'admin ui', as we expect that to be
    // used for the entity form callback.
    $entity_info = entity_get_info($this->entityType);
    if (isset($entity_info['admin ui']['file'])) {
      $item['file'] = $entity_info['admin ui']['file'];
    }
    if (isset($entity_info['admin ui']['file path'])) {
      $item['file path'] = $entity_info['admin ui']['file path'];
    }
    else {
      $item['file path'] = drupal_get_path('module', $entity_info['module']);
    }

    // Add in what the parent class has.
    $item += parent::menu_item($operation_path, $operation_definition);
    return $item;
  }

}

/**
 * Op handler for editing an entity: EntityAPI form.
 */
class EntityOperationsOperationEdit extends EntityOperationsOperationForm {

  public $access_verb = 'edit';
  // Title: "Edit %entity".

  /**
   * Properties for the menu item specific to this handler.
   *
   * Override the page callback to go straight to entity_ui_get_form().
   */
  function menu_item($operation_path, $operation_definition) {
    $item = array(
      'page callback' => 'entity_ui_get_form',
      'page arguments' => array($this->entityType, $this->controller->loader_position),
    );

    // Use the file setting from entity info 'admin ui', as we expect that to be
    // used for the entity form callback.
    $entity_info = entity_get_info($this->entityType);
    if (isset($entity_info['admin ui']['file'])) {
      $item['file'] = $entity_info['admin ui']['file'];
    }
    if (isset($entity_info['admin ui']['file path'])) {
      $item['file path'] = $entity_info['admin ui']['file path'];
    }
    else {
      $item['file path'] = drupal_get_path('module', $entity_info['module']);
    }

    // Add in what the parent class has.
    $item += parent::menu_item($operation_path, $operation_definition);
    return $item;
  }

}

/**
 * Generic op handler for action-ish operations.
 *
 * Operations exposed to Views Bulk Operations should (probably?) use this as
 * their parent class.
 *
 * This has a form which consists of just a button. Subclasses should override:
 *  - operationStrings() to provide texts specific to their operation
 *  - operationAccess() to determine when the operation is available
 *  - execute() to customize what happens.
 * If the action requires configuration, then subclasses should override:
 *  - formBody() to provide the form without the submit button
 *  - formSubmitGetContext() to process the form values into a context array.
 */
abstract class EntityOperationsOperationAction extends EntityOperationsOperationForm {

  /**
   * Returns basic information about the operation.
   *
   * @return
   *  In addition to the parent method's return, this can also include:
   *  - 'vbo': An array of options for the VBO operation.
   */
  function operationInfo() {
    return array(
      'provides tab' => FALSE,
      'uses form' => TRUE,
      'provides vbo' => TRUE,
      // Extra info for VBO.
      'vbo' => array(),
    );
  }

  /**
   * Returns strings for the operations.
   *
   * These should be translated, and the following placeholders may be used
   * without values:
   *  '%entity-type': The human-readable label for the entity type.
   *  '%label': The entity label.
   *
   * @return
   *  An array containing the following keys:
   *  - 'button label': the label for the form button. If used as a VBO, this
   *      should not contain the '%label' placeholder.
   *  - 'confirm question': the question to ask the user in the form.
   *  - 'submit message': the message shown to convey to the user that the
   *      action has succeeded.
   */
  function operationStrings() {
    return array(
      'button label' => t('Perform action'),
      'confirm question' => t('Are you sure you want to proceed?'),
      'submit message' => t('Action completed on %entity-type %label.', array(
        // This intentionally has no replacements for the placeholders; these
        // are replaced in getOperationString().
      )),
    );
  }

  /**
   * Get an operations string.
   *
   * @param $string_key
   *  The key in the array in operationStrings() for the string to retrieve.
   * @param $entity_type
   *  The entity type.
   * @param $entity
   *  The entity. Pass NULL if there is no entity in context.
   * @param $operation_path
   *  The key for the operation.
   *
   * @return
   *  A translated string, with replacements made.
   */
  function getOperationString($string_key, $entity_type, $entity, $operation_path) {
    $entity_info = entity_get_info($entity_type);
    $strings = $this->operationStrings();
    $substitutions = array(
      '%entity-type' => $entity_info['label'],
      '%label' => isset($entity) ? $entity->label() : '',
    );
    // Run the string through format_string() to replace the placeholders
    // which t() will have ignored.
    return format_string($strings[$string_key], $substitutions);
  }

  /**
   * Return the form body, that is, the form without the submit button.
   *
   * This allows this form to work either as standalone, or as a configuration
   * form in VBO.
   */
  function formBody($form, &$form_state, $entity_type, $entity, $operation_path) {
    return $form;
  }

  /**
   * Form builder for this operation.
   *
   * Subclasses should not override this, but formBody() instead.
   */
  function form($form, &$form_state, $entity_type, $entity, $operation_path) {
    $form += $this->formBody($form, &$form_state, $entity_type, $entity, $operation_path);

    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => $this->getOperationString('button label', $entity_type, $entity, $operation_path),
    );

    return $form;
  }

  /**
   * Form submit handler for this operation.
   *
   * Subclasses should not override this, but formSubmitGetContext() instead.
   */
  function formSubmit($form, &$form_state, $entity_type, $entity, $operation_path) {
    $context = $this->formSubmitGetContext($form, &$form_state, $entity_type, $entity, $operation_path);

    $this->execute($entity_type, $entity, $context);

    $message = $this->getOperationString('submit message', $entity_type, $entity, $operation_path);
    drupal_set_message($message);

    $form_state['redirect'] = $this->redirect_entity_uri($entity_type, $entity);
  }

  /**
   * Build a context array from the form values.
   *
   * @return
   *  A context array. This is passed to the execute() method.
   */
  function formSubmitGetContext($form, &$form_state, $entity_type, $entity, $operation_path) {
    $context = array(
      // This is just an example.
      //  'param' => TRUE,
    );

    return $context;
  }

  /**
   * Action callback.
   *
   * This receives an extra parameter consisting of extra data from the form
   * submission. This allows operations to be used in a generic way with VBO.
   */
  function execute($entity_type, $entity, $context = array()) {
  }

}

/**
 * Op handler for publishing an entity.
 *
 * Requires the entity to have a 'status' property.
 */
class EntityOperationsOperationPublish extends EntityOperationsOperationAction {

  // TODO: 'publish' usually requires stronger access than 'edit'.
  public $access_verb = 'edit';
  // Title: "Edit %entity".

  /**
   * Returns strings for the operations.
   *
   * @return
   *  An array containing the following keys:
   *  - 'form': An array of strings for the operation form, containing:
   *    - 'button label'
   *    - 'confirm question'
   *    - 'submit message'
   */
  function operationStrings() {
    return array(
      'button label' => t('Publish'),
      'confirm question' => t('Are you sure you want to publish this %entity-type?'),
      'submit message' => t('%entity-type %label has been published.'),
    );
  }

  /**
   * Access callback: deny access if entity is unpublished.
   */
  function operationAccess($entity_type, $entity) {
    // Deny access if the entity is already published.
    $published = $entity->status;
    // We check the node is not unpublished rather than whether it is published,
    // so modules that extend the status property can also use this (assuming
    // that they don't want entities with a status higher than 1 to revert to
    // merely being published).
    if ($published != NODE_NOT_PUBLISHED) {
      return FALSE;
    }
    // We only deny access; entity_access() will take over.
  }

  /**
   * The publish action.
   */
  function execute($entity_type, $entity, $context = array()) {
    $entity->status = NODE_PUBLISHED;
    $entity->save();
  }

}

/**
 * Op handler for publishing an entity.
 *
 * Requires the entity to have a 'status' property.
 */
class EntityOperationsOperationUnPublish extends EntityOperationsOperationPublish {

  /**
   * Returns strings for the operations.
   *
   * @return
   *  An array containing the following keys:
   *  - 'form': An array of strings for the operation form, containing:
   *    - 'button label'
   *    - 'confirm question'
   *    - 'submit message'
   */
  function operationStrings() {
    return array(
      'button label' => t('Unpublish'),
      'confirm question' => t('Are you sure you want to unpublish this %entity-type?'),
      'submit message' => t('%entity-type %label has been unpublished.'),
    );
  }

  /**
   * Access callback: deny access if entity is unpublished.
   */
  function operationAccess($entity_type, $entity) {
    // Deny access if the entity is already published.
    $published = $entity->status;
    // Use the constant, which allows module that extend the status property to
    // also use this, provided 'published' is 1.
    if ($published == NODE_NOT_PUBLISHED) {
      return FALSE;
    }
    // We only deny access; entity_access() will take over.
  }

  /**
   * The publish action.
   *
   * TODO: figure out a way of exposing this to:
   *  - hook_action_info
   *  - UI actions in admin list
   *  - VBO??
   */
  function execute($entity_type, $entity, $context = array()) {
    $entity->status = NODE_NOT_PUBLISHED;
    $entity->save();
  }

}
