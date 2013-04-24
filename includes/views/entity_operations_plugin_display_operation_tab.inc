<?php

/**
 * The plugin that handles entity tab views.
 *
 * Large portions of this are cribbed from EVA.
 *
 * @todo:
 *  - add validation to ensure that the op name set by the user doesn't clobber
 *    an existing one. Unless this is desirable behaviour?
 */
class entity_operations_plugin_display_operation_tab extends views_plugin_display {

  /**
   * The tab display has a path.
   */
  function has_path() { return TRUE; }

  /**
   * Return the base path to use for this display.
   *
   * This can be overridden for displays that do strange things
   * with the path. Like us! ;)
   */
  function get_path() {
    // @todo We probably should be doing things to return the actual path here,
    // but I'm not sure of the situations this gets called yet!
    return $this->get_option('path');
  }

  /**
   * Get the arguments for views_access() for this display.
   *
   * Cribbed from parts of execute_hook_menu().
   */
  function get_access_parameters() {
    $access_plugin = $this->get_plugin('access');
    if (!isset($access_plugin)) {
      $access_plugin = views_get_plugin('access', 'none');
    }

    // Get access callback might return an array of the callback + the dynamic arguments.
    $access_plugin_callback = $access_plugin->get_access_callback();

    if (is_array($access_plugin_callback)) {
      $access_arguments = array();

      // Find the plugin arguments.
      $access_plugin_method = array_shift($access_plugin_callback);
      $access_plugin_arguments = array_shift($access_plugin_callback);
      if (!is_array($access_plugin_arguments)) {
        $access_plugin_arguments = array();
      }

      $access_arguments[0] = array($access_plugin_method, &$access_plugin_arguments);

      // Move the plugin arguments to the access arguments array.
      $i = 1;
      foreach ($access_plugin_arguments as $key => $value) {
        if (is_int($value)) {
          $access_arguments[$i] = $value;
          $access_plugin_arguments[$key] = $i;
          $i++;
        }
      }
    }
    else {
      $access_arguments = array($access_plugin_callback);
    }

    return $access_arguments;
  }

  /**
   * Returns properties for this tab's menu item.
   */
  function menu_item() {
    $weight = $this->get_option('menu_weight');
    return array(
      'weight' => empty($weight) ? 0 : $weight,
    );
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['entity_type'] = array('default' => '');
    $options['path'] = array('default' => '');
    $options['menu_weight'] = array('default' => 10);
    $options['argument_mode'] = array('default' => 'id');
    $options['default_argument'] = array('default' => '');
    $options['show_title'] = 0;

    return $options;
  }

  /**
   * Provide the summary for page options in the views UI.
   *
   * This output is returned as an array.
   */
  function options_summary(&$categories, &$options) {
    // It is very important to call the parent function here:
    parent::options_summary($categories, $options);

    $categories['entity_view'] = array(
      'title' => t('Entity tab settings'),
      'column' => 'second',
      'build' => array(
        '#weight' => -10,
      ),
    );

    if ($entity_type = $this->get_option('entity_type')) {
      $entity_info = entity_get_info($entity_type);
      $type_name = $entity_info['label'];
      $bundle_names = array();
      foreach ($this->get_option('bundles') as $bundle) {
        $bundle_names[] = $entity_info['bundles'][$bundle]['label'];
      }
    }

    $options['entity_type'] = array(
      'category' => 'entity_view',
      'title' => t('Entity type'),
      'value' => empty($type_name) ? t('None') : $type_name,
    );

    $operation_name = $this->get_option('path');
    $options['operation_name'] = array(
      'category' => 'entity_view',
      'title' => t('Operation name'),
      'value' => empty($operation_name) ? t('None') : check_plain($operation_name),
    );

    $argument_mode = $this->get_option('argument_mode');
    $options['arguments'] = array(
      'category' => 'entity_view',
      'title' => t('Arguments'),
      'value' => empty($argument_mode) ? t('None') : check_plain($argument_mode),
    );

    if (module_exists('token')) {
      // We must load token values here to show them on the options form.
      drupal_add_js(drupal_get_path('module', 'token') . '/token.js');
      drupal_add_css(drupal_get_path('module', 'token') . '/token.css');
      drupal_add_library('token', 'treeTable');
    }
  }

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_form($form, $form_state);

    $entity_info = entity_get_info();
    $entity_type = $this->get_option('entity_type');

    switch ($form_state['section']) {
      case 'entity_type':
        foreach ($entity_info as $type => $info) {
          if (!empty($info['view modes'])) {
            $entity_names[$type] = $info['label'];
          }
        }

        $form['#title'] .= t('Entity type');
        $form['entity_type'] = array(
          '#type' => 'radios',
          '#required' => TRUE,
          '#title' => t("Attach this display to the following entity type"),
          '#options' => $entity_names,
          '#default_value' => $this->get_option('entity_type'),
        );
        break;

      case 'operation_name':
        // This is actually the operation name, but validate() will complain
        // if this is not around.
        $form['path'] = array(
          '#type' => 'textfield',
          '#required' => TRUE,
          '#title' => t("Path component"),
          '#description' => t('The path below the entity where this tab should appear.'),
          '#default_value' => $this->get_option('path'),
        );
        $form['menu_weight'] = array(
          '#title' => t('Weight'),
          '#type' => 'textfield',
          '#size' => 4,
          '#default_value' => $this->get_option('menu_weight'),
          '#description' => t('The lower the weight the higher/further left it will appear.'),
        );

        break;

      case 'arguments':
        $form['#title'] .= t('Arguments');
        $default = $this->get_option('argument_mode');
        $options = array(
          'none' => t("No special handling"),
          'id' => t("Use the ID of the entity the view is attached to"),
          'token' => t("Use tokens from the entity the view is attached to"),
        );

        $form['argument_mode'] = array(
          '#type' => 'radios',
          '#title' => t("How should this display populate the view's arguments?"),
          '#options' => $options,
          '#default_value' => $default,
        );

        $form['token'] = array(
          '#type' => 'fieldset',
          '#title' => t('Token replacement'),
          '#collapsible' => TRUE,
          '#states' => array(
            'visible' => array(
              'input:[name=argument_mode]' => array('value' => 'token'),
            ),
          ),
        );

        $form['token']['default_argument'] = array(
          '#title' => t('Arguments'),
          '#type' => 'textfield',
          '#default_value' => $this->get_option('default_argument'),
          '#description' => t('You may use token replacement to provide arguments based on the current entity. Separate arguments with "/".'),
        );

        if (module_exists('token')) {
          $form['token']['tokens'] = array(
            '#theme' => 'token_tree',
            '#token_types' => array(token_get_entity_mapping('entity', $entity_type)),
          );
        }
        break;

      case 'show_title':
        $form['#title'] .= t('Show title');
        $form['show_title'] = array(
          '#type' => 'checkbox',
          '#title' => t('Show the title of the view above the attached view.'),
          '#default_value' => $this->get_option('show_title'),
        );
        break;
    }
  }

  function options_submit(&$form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_submit($form, $form_state);

    switch ($form_state['section']) {
      case 'entity_type':
        $new_entity = $form_state['values']['entity_type'];
        $old_entity = $this->get_option('entity_type');

        $this->set_option('entity_type', $new_entity);
        if ($new_entity != $old_entity) {
          // Each entity has its own list of bundles and view modes. If there's
          // only one on the new type, we can select it automatically. Otherwise
          // we need to wipe the options and start over.
          $new_entity_info = entity_get_info($new_entity);

          $new_bundle_keys = array_keys($new_entity_info['bundles']);
          $new_bundles = array();
          if (count($new_bundle_keys) == 1) {
            $new_bundles[] = $new_bundle_keys[0];
          }
          $this->set_option('bundles', $new_bundles);
        }
        break;
      case 'operation_name':
        $this->set_option('path', $form_state['values']['path']);
        $this->set_option('menu_weight', $form_state['values']['menu_weight']);
        break;
      case 'arguments':
        $this->set_option('argument_mode', $form_state['values']['argument_mode']);
        if ($form_state['values']['argument_mode'] == 'token') {
          $this->set_option('default_argument', $form_state['values']['default_argument']);
        }
        else {
          $this->set_option('default_argument', NULL);
        }
        break;
      case 'show_title':
        $this->set_option('show_title', $form_state['values']['show_title']);
        break;
    }
  }

  /**
   * Enforce at least one 'empty' handler so the user never sees an empty tab.
   */
  function validate() {
    $errors = parent::validate();

    $empty_handlers = $this->get_handlers('empty');
    if (empty($empty_handlers)) {
      $errors[] = t('Display "@display" must have at least one "No results" behavior.', array(
        '@display' => $this->display->display_title,
      ));
    }

    return $errors;
  }

  /**
   * Cribbed from EVA.
   *
   * We have to run token replacement before the arguments are used.
   */
  function pre_execute() {
    // Call the parent setup function so we do not lose data.
    parent::pre_execute();

    if (isset($this->view->current_entity)) {
      $entity = $this->view->current_entity;
      $entity_type = $this->view->display_handler->get_option('entity_type');
      $entity_info = entity_get_info($entity_type);

      $arg_mode = $this->view->display_handler->get_option('argument_mode');
      if ($arg_mode == 'token') {
        if ($token_string = $this->view->display_handler->get_option('default_argument')) {
          // Now do the token replacement.
          $token_values = eva_get_arguments_from_token_string($token_string, $entity_type, $entity);
          $new_args = array();
          // We have to be careful to only replace arguments that have tokens.
          foreach ($token_values as $key => $value) {
            $new_args[$key] = $value;
          }

          $this->view->args = $new_args;
        }
      }
      elseif ($arg_mode == 'id') {
        $this->view->args = array($entity->{$entity_info['entity keys']['id']});
      }
    }
  }

  /**
   * Execute the view.
   */
  function execute() {
    // Prior to this being called, the $view should already be set to this
    // display, and arguments should be set on the view.
    if (!isset($this->view->override_path)) {
      $this->view->override_path = $_GET['q'];
    }

    $data = $this->view->render();
    if (!empty($this->view->result) || $this->get_option('empty') || !empty($this->view->style_plugin->definition['even empty'])) {
      return $data;
    }
  }

}
