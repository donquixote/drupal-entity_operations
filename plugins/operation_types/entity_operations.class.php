<?php

/**
 * @file
 * Defines the class for Entity Operation actions.
 * Belongs to the "action" operation type plugin.
 */

class EntityOperationsVBOOperations extends ViewsBulkOperationsBaseOperation {
// TODO: most of this is copied over from VBO: needs work!

  /**
   * Contains the options provided by the user in the configuration form.
   *
   * @var array
   */
  public $formOptions = array();

  /**
   * Returns the access bitmask for the operation, used for entity access checks.
   * TODO!
   */
  public function getAccessMask() {
    // Assume edit by default.
    if (!isset($this->operationInfo['behavior'])) {
      $this->operationInfo['behavior'] = array('changes_property');
    }

    $mask = 0;
    if (in_array('views_property', $this->operationInfo['behavior'])) {
      $mask |= VBO_ACCESS_OP_VIEW;
    }
    if (in_array('changes_property', $this->operationInfo['behavior'])) {
      $mask |= VBO_ACCESS_OP_UPDATE;
    }
    if (in_array('creates_property', $this->operationInfo['behavior'])) {
      $mask |= VBO_ACCESS_OP_CREATE;
    }
    if (in_array('deletes_property', $this->operationInfo['behavior'])) {
      $mask |= VBO_ACCESS_OP_DELETE;
    }
    return $mask;
  }

  /**
   * Returns whether the provided account has access to execute the operation.
   * TODO!
   *
   * @param $account
   */
  public function access($account) {
    // Use actions_permissions if enabled.
    if (module_exists('actions_permissions')) {
      $perm = actions_permissions_get_perm($this->operationInfo['label'], $this->operationInfo['key']);
      if (!user_access($perm, $account)) {
        return FALSE;
      }
    }
    // Check against additional permissions.
    if (!empty($this->operationInfo['permissions'])) {
      foreach ($this->operationInfo['permissions'] as $perm) {
        if (!user_access($perm, $account)) {
          return FALSE;
        }
      }
    }
    // Access granted.
    return TRUE;
  }

  /**
   * Returns whether the operation needs the full selected views rows to be
   * passed to execute() as a part of $context.
   */
  public function needsRows() {
    return !empty($this->operationInfo['pass rows']);
  }

  /**
   * Executes the selected operation on the provided data.
   *
   * @param $data
   *   The data to to operate on. An entity or an array of entities.
   * @param $context
   *   An array of related data (selected views rows, etc).
   */
  public function execute($data, array $context) {
    $handler_class = $this->operationInfo['handler'];
    $operation_handler = new $handler_class($this->entityType);

    $data = is_array($data) ? $data : array($data);
    foreach ($data as $entity) {
      // We always call execute, even if it doesn't do anything.
      $operation_handler->execute($this->entityType, $entity);
    }
  }

}
