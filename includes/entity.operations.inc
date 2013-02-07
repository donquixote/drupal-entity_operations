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

  public function __construct($entity_type, $entity_info) {
    $this->entityType = $entity_type;
    $this->entityInfo = $entity_info;
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
    // This allows altering of operations and adding of extra settings, without
    // either stashing excessive data in hook_entity_info(), or needing to
    // subclass this controller.
    $entity_operations = $this->entityInfo['operations ui']['operations'];
    drupal_alter('entity_operations_menu_operations', $entity_operations, $this->entityType);

    foreach ($entity_operations as $operation_path => $operation_definition) {
      $handler_class = $operation_definition['handler'];

      $operation_handler = new $handler_class($this->entityType);
      // Some handlers need to get some of our properties.
      // TODO: this is a bit ugly.
      $operation_handler->controller = $this;

      $operation_handler_info = $operation_handler->operationInfo();

      // Skip operations that don't provide a menu item, though if the
      // definition array contains a 'menu_item' property, we allow that to
      // override this, and assume the entity type definition knows what it is
      // doing!
      if (empty($operation_handler_info['provides menu']) && !isset($operation_definition['menu_item'])) {
        continue;
      }

      // Several things get to contribute properties to the menu item.
      $menu_item = array();
      // Allow the operation to go first. These should probably be set using
      // hook_entity_operations_operations_alter() rather than in
      // hook_entity_info(), as data from the latter is always loaded.
      if (isset($operation_definition['menu_item'])) {
        $menu_item = $operation_definition['menu_item'];
      }

      // Next, allow the handler to provide properties.
      $menu_item += $operation_handler->menu_item($operation_path, $operation_definition, $this->loader_position);

      // Finally, add in our defaults.
      $menu_item += array(
        'title' => ucfirst($operation_path),
        'type' => isset($operation_definition['default']) ? MENU_DEFAULT_LOCAL_TASK : MENU_LOCAL_TASK,
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
      if (isset($operation_definition['default'])) {
        $base_item = $menu_item;
        $base_item['title'] = 'View'; // Won't get used.
        $base_item['type'] = MENU_NORMAL_ITEM;

        $items[$this->base_path] = $base_item;
      }
    }

    // Invoke hook_entity_operations_operations_menu_alter().
    drupal_alter('entity_operations_operations_menu', $items, $this->entityType);

    return $items;
  }

}
