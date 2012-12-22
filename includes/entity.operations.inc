<?php

/**
 * Controller class for the entity operations UI.
 *
 * Registers paths with hook_menu(), based on entity info properties.
 */
class EntityOperationsDefaultUIController {

  protected $entityType;
  protected $entityInfo;
  // Argh, public because of the op handlers.
  public $path;

  public function __construct($entity_type, $entity_info) {
    $this->entityType = $entity_type;
    $this->entityInfo = $entity_info;
    $this->path = $this->entityInfo['operations ui']['path'];
  }

  public function hook_menu() {
    $items = array();

    // TODO: set these in __construct() so subclasses can override them before
    // we do our work here??
    $this->wildcard = isset($this->entityInfo['operations ui']['menu wildcard']) ? $this->entityInfo['operations ui']['menu wildcard'] : '%entity_object';
    $this->base_path = $this->path . '/' . $this->wildcard;
    $this->loader_position = count(explode('/', $this->path));

    $this->weight = 0;

    foreach ($this->entityInfo['operations ui']['operations'] as $operation_path => $operation_info) {
      $operation_class = $operation_info['handler'];
      $operation_handler = new $operation_class($this->entityType, $this);

      // Allow the handler to provide some properties first.
      $menu_item = $operation_handler->menu_item($operation_path, $operation_info);
      // Add in our defaults.
      $menu_item += array(
        'title' => ucfirst($operation_path),
        'type' => isset($operation_info['default']) ? MENU_DEFAULT_LOCAL_TASK : MENU_LOCAL_TASK,
        'load arguments' => array($this->entityType),
        'access callback' => 'entity_access',
        // Note that while entity_access() documents itself as only supporting
        // basic values of $op, in fact it just passes them to the access
        // callback, meaning that as long as your entity's access callback
        // understands it, you can do it here.
        'access arguments' => array($operation_handler->access_verb, $this->entityType),
        'weight' => $this->weight,
        // Very rarely, handlers may wish to set this (such as 'add'), so we
        // put this in the menu item to let them override it.
        'path' => $this->base_path . '/' . $operation_path,
      );

      // Get the path out of the menu item and remove it.
      $path = $menu_item['path'];
      unset($menu_item['path']);

      // Set our menu item.
      $items[$path] = $menu_item;

      $this->weight++;

      // The defaut operation also gets to be the parent tab.
      if (isset($operation_info['default'])) {
        $base_item = $menu_item;
        $base_item['title'] = t('Clients');
        $base_item['type'] = MENU_NORMAL_ITEM;

        $items[$this->base_path] = $base_item;
      }
    }


    return $items;
  }

}

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
   * Returns whether or not this handler provides a form.
   */
  function is_form() {
    return FALSE;
  }

  /**
   * Only needs the controller when used by the controller.
   * TODO!!! URRRGH!!! UGLY PATTERN!
   */
  public function __construct($entity_type, $ui_controller = NULL) {
    $this->entityType = $entity_type;
    $this->controller = $ui_controller;
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
  function menu_item($operation_path, $operation_info) {
    return array(
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
    );
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
   * Returns whether or not this handler provides a form.
   */
  function is_form() {
    return TRUE;
  }

  /**
   * Properties for the menu item specific to this handler.
   */
  function menu_item($operation_path, $operation_info) {
    return array(
      'page callback' => 'drupal_get_form',
      'page arguments' => array(
        'entity_operations_operation_form',
        $this->entityType,
        get_class($this),
        $this->controller->loader_position,
      ),
    );
  }

  /**
   * Form builder for this operation.
   *
   * To get different parameters, override menu_item().
   */
  function form($form, &$form_state, $entity_type, $entity) {
  }

  /**
   * Form validation handler for this operation.
   *
   * Receives the same parameters as the form builder as a convenience.
   */
  function formValidate($form, &$form_state, $entity_type, $entity) {
  }

  /**
   * Form submit handler for this operation.
   *
   * Receives the same parameters as the form builder as a convenience.
   */
  function formSubmit($form, &$form_state, $entity_type, $entity) {
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
 */
class EntityOperationsOperationAdd extends EntityOperationsOperationBase {

  public $access_verb = 'create';
  // Title: "Edit %entity".

  /**
   * Properties for the menu item specific to this handler.
   */
  function menu_item($operation_path, $operation_info) {
    return array(
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
    );
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
  function menu_item($operation_path, $operation_info) {
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

    return $item;
  }

}

/**
 * Op handler for publishing an entity.
 *
 * Requires the entity to have a 'status' property.
 */
class EntityOperationsOperationPublish extends EntityOperationsOperationForm {

  // TODO: 'publish' usually requires stronger access than 'edit'.
  public $access_verb = 'edit';
  // Title: "Edit %entity".

  /**
   * Properties for the menu item specific to this handler.
   *
   * Override the page callback to go straight to entity_ui_get_form().
   */
  function menu_item($operation_path, $operation_info) {
    // TODO: make this a callback rather than a tab??
    $item = array(
      'access callback' => 'entity_operations_operation_access_callback',
      'access arguments' => array($this->entityType, get_class($this), $this->controller->loader_position),
    );

    return $item;
  }

  /**
   * Menu access callback: deny access if entity is unpublished.
   */
  function menuAccess($entity_type, $entity) {
    // Deny access if the entity is already published.
    $published = $entity->status;
    if (!$published) {
      return FALSE;
    }

    // Let entity access take over.
    return entity_access($this->access_verb, $this->entityType);
  }

  /**
   * Form builder for this operation.
   *
   * To get different parameters, override menu_item().
   */
  function form($form, &$form_state, $entity_type, $entity) {
    drupal_set_title(t('Publish %entity', array(
      '%entity' => $entity->label(),
    )), PASS_THROUGH);

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Publish'),
    );

    return $form;
  }

  /**
   * Form submit handler for this operation.
   *
   * Receives the same parameters as the form builder as a convenience.
   */
  function formSubmit($form, &$form_state, $entity_type, $entity) {
    $this->action($entity_type, $entity);
    $entity_info = entity_get_info($entity_type);
    drupal_set_message(t('%entity-type %label has been published.', array(
      '%entity-type' => $entity_info['label'],
      '%label' => $entity->label(),
    )));
    $form_state['redirect'] = $this->redirect_entity_uri($entity_type, $entity);
  }

  /**
   * The publish action.
   *
   * TODO: figure out a way of exposing this to:
   *  - hook_action_info
   *  - UI actions in admin list
   *  - VBO??
   */
  function action($entity_type, $entity) {
    $entity->status = TRUE;
    $entity->save();
  }

}
