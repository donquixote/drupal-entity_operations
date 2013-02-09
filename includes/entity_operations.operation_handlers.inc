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
 *    - EntityOperationsOperationPageViews: output the result of a Views
 *      entity tab display.
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

  /**
   * The operation for entity_access().
   *
   * Subclasses can set this to NULL to allow the path component to determine
   * the access verb.
   *
   * @see getAccessVerb()
   */
  protected $access_verb = 'view';

  /**
   * The path component this operation is on.
   */
  protected $path;

  protected $entityType;

  /**
   * The entity info.
   */
  protected $entityInfo;

  /**
   * The operations controller. Only set when building hook_menu().
   */
  public $controller;

  /**
   * Returns basic information about the operation.
   *
   * @return
   *  An array with the following keys:
   *  - 'provides menu': Whether this operation should provide a menu item on
   *    the entity base path.
   *  - 'uses form': If 'provides menu' is TRUE, whether this operation's tab
   *    is a form rather than a page.
   *  - 'provides views field': Whether this operation should be exposed as a
   *    Views field.
   *  - 'provides vbo': Whether this operation should be exposed to Views Bulk
   *    Operations.
   *  - 'provides services resource': Whether this operation should be exposed
   *    to Services as a targeted action on the entity resource.
   * Note that the 'provides vbo' and 'provides services resource' properties
   * should be used with EntityOperationsOperationAction or a subclass.
   */
  function operationInfo() {
    return array(
      // Default behaviour is to provide a menu item.
      'provides menu' => TRUE,
      'uses form' => FALSE,
      'provides vbo' => FALSE,
      'provides views field' => TRUE,
    );
  }

  /**
   * Constructor for operation handler objects.
   *
   * @param $entity_type
   *  The entity type.
   * @param $operation_path
   *  The path component for the operation.
   */
  public function __construct($entity_type, $operation_path) {
    $this->entityType = $entity_type;
    $this->path = $operation_path;
  }

  /**
   * Get the access verb for this operation.
   *
   * This is taken either from the property $access_verb, or if that is NULL,
   * the operation path component.
   *
   * TODO: revisit this logic and see if anything better can be devised.
   *
   * @return
   *  The access verb for the operation, e.g. 'view', 'edit', and so on.
   */
  public function getAccessVerb() {
    $verb = isset($this->access_verb) ? $this->access_verb : $this->path;
    return $verb;
  }

  /**
   * Properties for the menu item specific to this handler.
   *
   * @param $operation_path
   *  The path component for the operation, e.g., 'view', 'edit'.
   * @param $operation_definition
   *  The definition array for the operation, from the operations definitions
   *  in hook_entity_info().
   * @param $loader_position
   *  The position of the entity loader in the menu base path.
   *
   * @return
   *  An array of items suitable for hook_menu() for this operation. See
   *  EntityOperationsDefaultUIController::hook_menu() for default values.
   */
  function menu_item($operation_path, $operation_definition, $loader_position) {
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
   *  granted.
   */
  function userAccess($entity_type, $entity, $account = NULL) {
    // We use entity_access() by default.
    // Note that while entity_access() documents itself as only supporting
    // basic values of $op, in fact it just passes them to the access
    // callback, meaning that as long as your entity's access callback
    // understands it, you can do it here.
    // @see entity_operations_get_entity_permissions().
    $verb = $this->getAccessVerb();
    $access = entity_access($verb, $entity_type, $entity, $account);

    return $access;
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
  function menu_item($operation_path, $operation_definition, $loader_position) {
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
        $operation_path,
        $loader_position,
      ),
      'access callback' => 'entity_operations_operation_access_callback',
      'access arguments' => array(
        $this->entityType,
        get_class($this),
        $operation_path,
        $loader_position,
      ),
    );

    // Add in what the parent class has.
    $item += parent::menu_item($operation_path, $operation_definition, $loader_position);
    return $item;
  }

  /**
   * Page callback.
   *
   * @param $entity_type
   *  The entity type.
   * @param $entity
   *  The entity. Pass NULL if there is no entity in context.
   * @param $params
   *  (optional) An array of additional parameters. It is up to subclasses that
   *  require this to document what they expect.
   *
   * @return
   *  Output suitable for a page callback.
   */
  function page($entity_type, $entity, $params = array()) {
    return 'This is my operation page.';
  }

}

/**
 * Handler for showing a Views display as a tab.
 *
 * @see entity_operations_plugin_display_operation_tab
 */
class EntityOperationsOperationPageViews extends EntityOperationsOperationPage {

  /**
   * The operation for entity_access().
   *
   * TODO: allow the views display plugin to expose this as a user option?
   */
  public $access_verb = 'view';

  /**
   * Returns the menu item for this operation.
   *
   * We process the info from entity_operations_get_views() here so it can
   * be overridden easily.
   */
  function menu_item($operation_path, $operation_definition, $loader_position) {
    $item = array(
      // TODO: Either take the op name as tab title when the view has none, or
      // enforce a title in the Views display plugin.
      'title' => $operation_definition['views info']['title'],
      'page callback' => 'entity_operations_operation_page_callback', // ?
      'page arguments' => array(
        $this->entityType,
        // We need to pass $this to get_class() because we want the class of
        // the current object, not the class we are sitting in.
        get_class($this),
        $operation_path,
        $loader_position,
        // Extra params.
        array(
          'view_name' => $operation_definition['views info']['name'],
          'view_display_name' => $operation_definition['views info']['display'],
        ),
      ),
      'access callback' => 'entity_operations_operation_access_callback',
      'access arguments' => array(
        $this->entityType,
        get_class($this),
        $operation_path,
        $loader_position,
      ),
    );

    // Add in what the parent class has.
    $item += parent::menu_item($operation_path, $operation_definition, $loader_position);
    return $item;
  }

  /**
   * Page callback for outputting a View.
   *
   * Requires extra params in $params:
   *  - 'view_name': The name of the view to output.
   *  - 'view_display_name': The name of the view display to use.
   */
  function page($entity_type, $entity, $params = array()) {
    $view_name = $params['view_name'];
    $view_display_name = $params['view_display_name'];

    if ($view = views_get_view($view_name)) {
      $view->set_display($view_display_name);
      if ($view->access($view_display_name)) {
        $view->current_entity = $entity;

        $result = $view->execute_display($view_display_name);
      }
    }

    if (!empty($result)) {
      return $result;
    }
    else {
      // We have to return at least an empty string for the page to display.
      // Ideally, a view used with this should provide 'no results' text so we
      // never come here.
      // @todo: add validation to the views display handler to insist on this.
      return '';
    }
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
      'provides menu' => TRUE,
      'uses form' => TRUE,
      'provides vbo' => FALSE,
      'provides views field' => TRUE,
    );
  }

  /**
   * Returns strings for the operations.
   *
   * These should be translated, and the following placeholders may be used
   * without values:
   *  '%entity-type': The human-readable label for the entity type.
   *  '%label': The entity label.
   * If further substitutions are required, they can be provided by overriding
   * getOperationStringSubstitutions().
   *
   * @return
   *  An array containing the following keys:
   *  - 'button label': the label for the form button. If used as a VBO, this
   *      should not contain the '%label' placeholder. TODO: not in this class!?
   *  - 'confirm question': the question to ask the user in the form. This may
   *      be blank if no confirmation question is ever required for the form.
   *  - 'submit message': the message shown to convey to the user that the
   *      action has succeeded. This may be blank if no submit message is
   *      required, or if an overridden formSubmit() does something else.
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
   * Properties for the menu item specific to this handler.
   */
  function menu_item($operation_path, $operation_definition, $loader_position) {
    return array(
      'page callback' => 'drupal_get_form',
      'page arguments' => array(
        'entity_operations_operation_form',
        $this->entityType,
        get_class($this),
        $operation_path,
        $loader_position, // Provides the entity as a parameter.
      ),
      'access callback' => 'entity_operations_operation_access_callback',
      'access arguments' => array(
        $this->entityType,
        get_class($this),
        $operation_path,
        $loader_position,
      ),
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
   * @param $parameters
   *  (Optional) The parameters array from the form submission values.
   *
   * @return
   *  A translated string, with replacements made.
   */
  function getOperationString($string_key, $entity_type, $entity, $operation_path, $parameters = array()) {
    $strings = $this->operationStrings();

    $substitutions = $this->getOperationStringSubstitutions($entity_type, $entity, $operation_path, $parameters);

    // Run the string through format_string() to replace the placeholders
    // which t() will have ignored.
    return format_string($strings[$string_key], $substitutions);
  }

  /**
   * Return an array of string substitutions for t().
   *
   * This may be overridden to add further substitutions, in particular using
   * the parameters array for the submit confirmation message.
   *
   * @param $entity_type
   *  The entity type.
   * @param $entity
   *  The entity. Pass NULL if there is no entity in parameters.
   * @param $operation_path
   *  The key for the operation.
   * @param $parameters
   *  The parameters array from the form submission values. When this is not
   *  available, for example when retrieving the button label, this will be an
   * empty array.
   *
   * @return
   *  An array of string substitutions as used by t(). These should match those
   *  used in operationStrings(), and should include at least:
   *  - '%entity-type': The human-readable label for the entity type.
   *  - '@entity-type': The human-readable label for the entity type.
   *  - '%label': The label of the entity.
   *  - '@label': The label of the entity.
   * We provide both the escaped and placeholder versions (i.e., @ and %)
   * because in some circumstances (such as buttons) the HTML included in the
   * placeholder is uunsuitable.
   */
  function getOperationStringSubstitutions($entity_type, $entity, $operation_path, $parameters) {
    $entity_info = entity_get_info($entity_type);

    // We provide each of these twice so store them in a variable.
    $entity_type = $entity_info['label'];
    $entity_label = isset($entity) ? $entity->label() : '';
    $substitutions = array(
      '%entity-type' => $entity_info['label'],
      '@entity-type' => $entity_info['label'],
      '%label' => $entity_label,
      '@label' => $entity_label,
    );
    return $substitutions;
  }

  /**
   * Helper to get the form for this operation.
   *
   * This wraps around form(), but allows various permutations of the form to
   * be returned. Additional elements can be requested by the caller by setting
   * the following array keys to TRUE in an array in
   * $form_state['entity_operation_form_elements']:
   *  - 'confirmation message': The confirmation question.
   *  - 'form elements': The body of the form as returned by form().
   *  - 'action button': The submit button.
   *  - 'cancel link': A link to return to the entity.
   */
  function getForm($form, &$form_state, $entity_type, $entity, $operation_path) {
    // Case 1: tab: always show everything.
    // Case 2: node view: show body and button (but not cancel link).
    // Case 3: VBO: show only body, if configurable.

    // Always show form elements.
    $form_state['entity_operation_form_elements'] += array(
      'form elements' => TRUE,
    );

    // Add confirmation message on tab.
    // TODO: ONLY if the form array is empty?
    // (though presumably if you don't want it, just don't set the message)
    if (!empty($form_state['entity_operation_form_elements']['confirmation message'])) {
      $message = $this->getOperationString('confirm question', $entity_type, $entity, $operation_path);
      if (!empty($message)) {
        $form['#attributes']['class'][] = 'confirmation';
        $form['question'] = array(
          '#prefix' => '<p>',
          '#markup' => $message,
          '#suffix' => '</p>',
        );
      }
    }

    if (!empty($form_state['entity_operation_form_elements']['form elements'])) {
      $form = $this->form($form, $form_state, $entity_type, $entity, $operation_path);
    }

    if (!empty($form_state['entity_operation_form_elements']['action button'])) {
      $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => $this->getOperationString('button label', $entity_type, $entity, $operation_path),
      );
    }
    if (!empty($form_state['entity_operation_form_elements']['cancel link'])) {
      $uri = entity_uri($entity_type, $entity);
      $form['actions']['cancel'] = array(
        '#type' => 'link',
        '#title' => t('Cancel'),
        '#href' => $uri['path'],
        '#options' => $uri['options'],
      );
    }

    // Add the actions element properties if it's needed.
    if (isset($form['actions'])) {
      $form['actions'] += array(
        '#type' => 'actions',
        '#weight' => 10,
      );
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

  function page($entity_type, $entity, $params = array()) {
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

  function page($entity_type, $entity, $params = array()) {
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
  function page($entity_type, $entity, $params = array()) {
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
      'provides menu' => TRUE,
      'uses form' => TRUE,
      'provides vbo' => FALSE,
      'provides views field' => FALSE,
    );
  }

  /**
   * Properties for the menu item specific to this handler.
   */
  function menu_item($operation_path, $operation_definition, $loader_position) {
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
        $operation_path,
        NULL,
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
    $item += parent::menu_item($operation_path, $operation_definition, $loader_position);
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
  function menu_item($operation_path, $operation_definition, $loader_position) {
    $item = array(
      'page callback' => 'entity_ui_get_form',
      'page arguments' => array($this->entityType, $loader_position),
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
    $item += parent::menu_item($operation_path, $operation_definition, $loader_position);
    return $item;
  }

}

/**
 * Generic handler for operations that are treated as actions.
 *
 * These are made available by default to:
 *  - Views Bulk Operations
 *  - Services, as targeted actions on an entity.
 *
 * This has a form which consists of just a button. Subclasses should override:
 *  - operationStrings() to provide texts specific to their operation
 *  - operationAccess() to determine when the operation is available
 *  - execute() to customize what happens.
 * If the action requires configuration, then subclasses should override:
 *  - form() to provide the form without the submit button
 *  - formSubmitGetParameters() to process the form values into a parameters
 *    array which is then passed to execute().
 *
 * See EntityOperationsOperationPublish for an example implementation using
 * just the basic form, and EntityOperationsOperationSetOwner for an example
 * requiring additional parameters.
 */
abstract class EntityOperationsOperationAction extends EntityOperationsOperationForm {

  /**
   * Returns basic information about the operation.
   *
   * @return
   *  In addition to the parent method's return, this can also include:
   *  - 'vbo': An array of options for the VBO operation.
   *  - 'parameters': An array describing the keys that execute() expects in its
   *    $parameters argument. The keys should be the key names that execute()
   *    expects, and the value should describe the type and be one of 'int',
   *    'string', or 'array'.
   */
  function operationInfo() {
    return array(
      'provides menu' => FALSE,
      'uses form' => TRUE,
      'provides vbo' => TRUE,
      // Extra info for VBO.
      'vbo' => array(),
      'provides views field' => FALSE,
      'provides services resource' => TRUE,
      'parameters' => array(),
    );
  }

  /**
   * Form builder for this operation.
   *
   * This should return the main part of the form, but without any of:
   *  - a confirmation question (if one is required)
   *  - a submit button
   *  - a cancel link
   * These are added as appropriate by getForm().
   */
  function form($form, &$form_state, $entity_type, $entity, $operation_path) {
    return $form;
  }

  /**
   * Form submit handler for this operation.
   *
   * Subclasses should not override this, but formSubmitGetParameters() instead.
   */
  function formSubmit($form, &$form_state, $entity_type, $entity, $operation_path) {
    // Set the operation parameters on the handler so that subclasses that *do*
    // override this can get to it.
    $this->parameters = $this->formSubmitGetParameters($form, $form_state, $entity_type, $entity, $operation_path);

    $this->execute($entity_type, $entity, $this->parameters);

    $message = $this->getOperationString('submit message', $entity_type, $entity, $operation_path, $this->parameters);
    if (!empty($message)) {
      drupal_set_message($message);
    }

    $form_state['redirect'] = $this->redirect_entity_uri($entity_type, $entity);
  }

  /**
   * Build a parameters array from the form values.
   *
   * @return
   *  An array of additional parameters for the operation's execute() method.
   */
  function formSubmitGetParameters($form, &$form_state, $entity_type, $entity, $operation_path) {
    $parameters = array(
      // This is just an example.
      //  'param' => TRUE,
    );

    return $parameters;
  }

  /**
   * Action callback.
   *
   * This receives an extra parameter consisting of extra data from the form
   * submission. This allows operations to be used in a generic way with VBO.
   */
  function execute($entity_type, $entity, $parameters = array()) {
  }

}

/**
 * Op handler for deleting an entity.
 */
class EntityOperationsOperationDelete extends EntityOperationsOperationAction {

  public $access_verb = 'delete';

  /**
   * Returns basic information about the operation.
   */
  function operationInfo() {
    return array(
      'provides menu' => TRUE,
      // Services module already provides CRUD operations for entities, so we
      // don't expose this to it.
      'provides services resource' => FALSE,
    ) + parent::operationInfo();
  }

  /**
   * Returns strings for the operations.
   */
  function operationStrings() {
    return array(
      'button label' => t('Delete @entity-type'),
      'confirm question' => t('Are you sure you want to delete the @entity-type %label?'),
      'submit message' => t('The @entity-type %label has been deleted.'),
      // These intentionally have no replacements for the placeholders; these
      // are replaced in getOperationString().
    );
  }

  /**
   * Form submit handler for this operation.
   *
   * Subclasses should not override this, but formSubmitGetParameters() instead.
   */
  function formSubmit($form, &$form_state, $entity_type, $entity, $operation_path) {
    parent::formSubmit($form, $form_state, $entity_type, $entity, $operation_path);

    // Overwrite the redirect, as there is no entity to return to.
    // Just send to the front page; there is nowhere else sensible to go to.
    $form_state['redirect'] = '<front>';
  }

  /**
   * The delete action.
   */
  function execute($entity_type, $entity, $parameters = array()) {
    $entity->delete();
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
  function execute($entity_type, $entity, $parameters = array()) {
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
  function execute($entity_type, $entity, $parameters = array()) {
    $entity->status = NODE_NOT_PUBLISHED;
    $entity->save();
  }

}

/**
 * Op handler for setting the owner/author of an entity.
 *
 * Requires the entity to have a 'uid' property.
 *
 * This is mostly provided as an example of a more complex action, and it should
 * be noted that this will currently fail the access check!
 */
class EntityOperationsOperationSetOwner extends EntityOperationsOperationAction {

  /**
   * Returns basic information about the operation.
   */
  function operationInfo() {
    return array(
      'provides menu' => TRUE,
      'vbo' => array(
        'configurable' => TRUE,
      ),
      'parameters' => array(
        'uid' => 'int',
      ),
    ) + parent::operationInfo();
  }

  /**
   * Returns strings for the operations.
   */
  function operationStrings() {
    return array(
      'button label' => t('Set author'),
      'confirm question' => t('Are you sure you want to set the author of this %entity-type?'),
      'submit message' => t('Owner of %entity-type %label has been set to !account-name.'),
    );
  }

  /**
   * Return an array of string substitutions for t().
   *
   * We add the themed username for the submit message.
   */
  function getOperationStringSubstitutions($entity_type, $entity, $operation_path, $parameters) {
    $substitutions = parent::getOperationStringSubstitutions($entity_type, $entity, $operation_path, $parameters);

    if (isset($parameters['uid'])) {
      $account = user_load($parameters['uid']);
      $substitutions['!account-name'] = theme('username', array('account' => $account));
    }

    return $substitutions;
  }

  /**
   * Return the form body, that is, the form without the submit button.
   */
  function form($form, &$form_state, $entity_type, $entity, $operation_path) {
    // TODO: default value.

    $form['owner'] = array(
      '#type' => 'textfield',
      '#title' => t('Owner'),
      '#autocomplete_path' => 'user/autocomplete',
      '#size' => '24',
      '#required' => TRUE,
      '#maxlength' => '60',
      '#description' => t('Enter the username to assign ownership of this entity to.'),
    );

    return $form;
  }

  /**
   * Build a parameters array from the form values.
   */
  function formSubmitGetParameters($form, &$form_state, $entity_type, $entity, $operation_path) {
    // Get the user out of the form value.
    $uid = db_query('SELECT uid from {users} WHERE name = :name', array(':name' => $form_state['values']['owner']))->fetchField();

    $parameters = array(
      'uid' => $uid,
    );

    return $parameters;
  }

  /**
   * Execute the operation.
   */
  function execute($entity_type, $entity, $parameters = array()) {
    $entity->uid = $parameters['uid'];
    $entity->save();
  }

}

/**
 * Op handler for Organic Groups group overview.
 */
class EntityOperationsOperationOGGroup extends EntityOperationsOperationPage {

  /**
   * Returns the menu item for this operation.
   */
  function menu_item($operation_path, $operation_definition, $loader_position) {
    $item = array(
      'title' => 'Group',
      // These won't work: these expect IDs not loaded entities. Hence we have
      // to define userAccess() and page() as wrappers.
      //'page callback' => 'og_ui_group_admin_overview',
      //'page arguments' => array($this->entityType, $loader_position),
      //'access callback' => 'og_ui_get_group_admin',
      //'access arguments' => array($this->entityType, $loader_position),
      'file path' => drupal_get_path('module', 'og_ui'),
      'file' => 'og_ui.admin.inc',
      'context' => MENU_CONTEXT_PAGE | MENU_CONTEXT_INLINE,
    );

    // Add in what the parent class has.
    $item += parent::menu_item($operation_path, $operation_definition, $loader_position);
    return $item;
  }

  /**
   * Determine access to the operation for the user.
   */
  function userAccess($entity_type, $entity, $account = NULL) {
    // This is just a wrapper.
    return og_ui_get_group_admin($entity_type, $entity->identifier());
  }

  /**
   * Page callback.
   *
   * We need this as a wrapper around og_ui_group_admin_overview() because
   * its parameters aren't compatible with our menu loaders.
   */
  function page($entity_type, $entity, $params = array()) {
    // This is just a wrapper.
    return og_ui_group_admin_overview($entity_type, $entity->identifier());
  }

}
