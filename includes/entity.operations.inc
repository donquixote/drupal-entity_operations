<?php

/**
 * Controller class for the entity operations UI.
 *
 * Registers paths with hook_menu(), based on entity info properties.
 *
 * The menu item returned for an operation is built up from, in order:
 *  - the operation array's 'menu item' property (if it exists),
 *  - the operation handler's menu_item() method,
 *  - default properties set in this controller's hook_menu().
 */
class EntityOperationsDefaultUIController {

  protected $entityType;
  protected $entityInfo;
  // Argh, public because of the op handlers.
  public $path;

  public function __construct($entity_type) {
    $this->entityType = $entity_type;

    $this->entityInfo = entity_get_info($this->entityType);
    $this->entityOperationInfo = entity_operations_get_operation_info($this->entityType);

    $this->path = $this->entityInfo['operations ui']['path'];
  }

  /**
   * Returns items for hook_menu() for an entity's operations.
   */
  public function hook_menu() {
    $items = array();

    // TODO: set these in __construct() so subclasses can override them before
    // we do our work here??
    $this->wildcard = isset($this->entityInfo['operations ui']['menu wildcard']) ? $this->entityInfo['operations ui']['menu wildcard'] : '%entity_object';
    $this->base_path = $this->path . '/' . $this->wildcard;
    $this->loader_position = count(explode('/', $this->path));

    $this->weight = 0;

    // Invoke hook_entity_operations_menu_operations_alter().
    // This allows altering of operations and adding operations which otherwise
    // would cause circularity.
    $entity_operations = $this->entityOperationInfo;
    drupal_alter('entity_operations_menu_operations', $entity_operations, $this->entityType);

    foreach ($entity_operations as $operation_path => $operation_definition) {
      // Only act on operations that request a menu item.
      // We use empty() so that the definition can set this to FALSE to
      // override handler info.
      if (empty($operation_definition['provision']['menu'])) {
        continue;
      }

      $handler_class = $operation_definition['handler'];
      $operation_handler = new $handler_class($this->entityType, $operation_path);
      // Some handlers need to get some of our properties.
      // TODO: this is a bit ugly.
      $operation_handler->controller = $this;

      $operation_handler_info = $operation_handler->operationInfo();

      // Several things get to contribute properties to the menu item.
      $menu_item = array();
      // Allow the operation to go first.
      if (!empty($operation_definition['provision']['menu']['menu item'])) {
        $menu_item = $operation_definition['provision']['menu']['menu item'];
      }

      // Next, allow the handler to provide properties.
      $menu_item += $operation_handler->menu_item($operation_path, $operation_definition, $this->loader_position);

      // Finally, add in our defaults.

      // Get the title for the menu item.
      $title = $operation_handler->getOperationString('tab title', $this->entityType, NULL, $operation_path);

      $menu_item += array(
        'title' => $title,
        'type' => isset($operation_definition['provision']['menu']['default']) ? MENU_DEFAULT_LOCAL_TASK : MENU_LOCAL_TASK,
        'load arguments' => array($this->entityType),
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

      // Increase the weight for the next menu item.
      $this->weight++;

      // The default operation also gets to be the parent tab.
      if (!empty($operation_definition['provision']['menu']['default'])) {
        $base_item = $menu_item;
        $base_item['title'] = 'View'; // Won't get used.
        $base_item['type'] = MENU_NORMAL_ITEM;

        $items[$this->base_path] = $base_item;
      }
    }

    return $items;
  }

}
