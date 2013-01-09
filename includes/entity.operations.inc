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

    foreach ($this->entityInfo['operations ui']['operations'] as $operation_path => $operation_definition) {
      $operation_class = $operation_definition['handler'];
      $operation_handler = new $operation_class($this->entityType, $this);
      $operation_info = $operation_handler->operationInfo();

      // Skip operations that don't provide a tab.
      if (empty($operation_info['provides tab'])) {
        continue;
      }

      // Allow the handler to provide some properties first.
      $menu_item = $operation_handler->menu_item($operation_path, $operation_definition);
      // Add in our defaults.
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
