<?php

/**
 * @file
 * Hooks provided by the Remote Entity API.
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
 *    - 'menu wildcard': A menu wildcard for loading the entity. This should
 *        be of the form '%foo' where foo_load() is a menu loader function.
 *        For example, for nodes this would be '%node'.
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
 *
 * Other required properties:
 *  - 'admin ui' must be set if using the add or edit operations.
 *    - 'file path' should be set, because it's consumed by another module.
 *  - 'module' must be set.
 *  - 'entity class' must be Entity or a subclass.
 *
 * @see hook_entity_info()
 */
function entity_opertations_hook_entity_info() {
}
