<?php

/**
 * @file
 * Hooks provided by the Entity Operations module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Provide an entity type via the remote entity API.
 *
 * This is a placeholder for describing further keys for hook_entity_info(),
 * which are introduced by the entity operations API for providing a UI for
 * an entity type.
 *
 * TODO: Move these to a dedicated hook, since this is only ever needed when
 * clearing menu cache and does not need to be loaded into memory with the
 * rest of entity info?
 *
 *  -'operations ui': An array containing the following properties:
 *    - 'controller class' (optional) The name of a class to use as a controller
 *      for the operations system. Defaults to
 *      EntityOperationsDefaultUIController.
 *    - 'path': The base path for an entity. This should not include the
 *      wildcard or the trailing slash. For example, for nodes this would
 *      be 'node'.
 *      Note that to avoid duplication of logic here and in the entity's URI
 *      callback, the 'entity uri' property may be set to the generic callback
 *      entity_operations_entity_uri(), which makes use of this base path.
 *    - 'menu wildcard': (optional) A menu wildcard for loading the entity. This
 *      defaults to '%entity_object', which should be suitable in most cases.
 *      If set, should be of the form '%foo' where foo_load() is a menu loader
 *      function. For example, for nodes this would be '%node'.
 *    - 'operations': An array of operations for the entity type. These are
 *      keyed by the operation machine name, which forms the path component
 *      after the entity. For example, defining operations 'view' and 'edit'
 *      would create paths 'base_path/%wildcard/view' and
 *      'base_path/%wildcard/edit'. Each operation array which may contain:
 *        - 'handler': The name of a handler class. For example, the standard
 *          edit handler class is 'EntityOperationsOperationEdit'.
 *        - 'default': (optional) Whether this operation is the default. Only
 *          one operation may be the default. This causes it to respond to the
 *          base URI of the entity, ie 'base_path/%wildcard'. For nodes, the
 *          'view' operation would be the default.
 *        - 'menu item': (optional) Properties to use for this operation's
 *          menu item.
 *
 * Other required properties:
 *  - 'module' must be set.
 *  - 'entity class' must be Entity or a subclass.
 *  - 'access callback' must be a callback that works with entity_access().
 *  - 'admin ui' must be set if using the add or edit operations.
 *    - 'file path' should be set, because it's consumed by another module.
 *    - 'controller' may be set to 'EntityOperationsDefaultAdminUIController'
 *      or a subclass, to override the default admin links on entities with
 *      those for operations.
 *
 * @see hook_entity_info()
 */
function entity_operations_hook_entity_info() {
}

/**
 * Alter menu items for an entity type's operations.
 *
 * This is called just before these are passed to hook_menu().
 *
 * @param $items
 *  The hook_menu() items.
 * @param $entity_type
 *  The entity type.
 */
function hook_entity_operations_operations_menu_alter(&$items, $entity_type) {

}