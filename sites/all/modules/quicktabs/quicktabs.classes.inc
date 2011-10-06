<?php

/**
 * A QuickSet object is an unrendered Quicktabs intance, as defined by a set of 
 * options and the contents it contains.
 */
class QuickSet {
  
  /**
   * The unique name of the QuickSet object.
   * This corresponds to the machine name as stored in the database or as defined
   * in code.
   * @var string
   */
  protected $name;
  
  /**
   * The contents array.
   * An array of objects that implement the QuickContentRenderable interface.
   * @var array
   */
  protected $contents;
  
  /**
   * An array of settings controlling the behaviour of the QuickSet object. See
   * the getDefaultOptions() static function of this class for the full list of
   * options.
   * @var array
   */
  protected $options;
  

  /**
   * Accessors.
   */
  
  public function getName() {
    return $this->name;
  }
  
  public function getContents() {
    return $this->contents;
  }
  
  public function getOptions() {
    return $this->options;
  }

  public function getTitle() {
    return isset($this->options['title']) ? $this->options['title'] : $this->name;
  }

  /**
   * Instantiate, populate and return a QuickSet object wrapped in a renderer.
   *
   * @param $name
   *   The unique name (machine name) of the QuickSet instance.
   *
   * @param $contents
   *   The array of content items, each one itself an array with at least a 'type'
   *   key, a title key, and the other info necessary for that type.
   *
   * @param $renderer
   *   The plugin key for this renderer plugin
   *
   * @param $options
   *   An array of options determining the behaviour of this QuickSet instance.
   *  
   */
  public static function QuickSetRendererFactory($name, $contents, $renderer, $options) {
    ctools_include('plugins');
    if ($class = ctools_plugin_load_class('quicktabs', 'renderers', $renderer, 'handler')) {
      try {
        $qs = new self($name, $contents, $options);
      }
      catch (InvalidQuickSetException $e) {
        watchdog('Quicktabs', $e->getMessage());
        return NULL;
      }
      return new $class($qs);
    }
  }
  
  /**
   * Returns a reference to an object that implements the QuickContentRenderable
   * interface.
   */
  public static function getContentRenderer($tab) {
    if ($tab['type'] == 'prerendered') {
      return new QuickPreRenderedContent($tab);
    }
    if ($content = QuickContent::factory($tab['type'], $tab)) return $content;
    return NULL;
  }
  
  /**
   * Static method to retrieve content from an ajax call. This is called by the
   * quicktabs_ajax() callback in quicktabs.module.
   */
  public static function ajaxRenderContent($type, $args) {
    if ($renderer = self::getContentRenderer(array('type' => $type))) {
      $output = $renderer->render(FALSE, $args);
      return !empty($output) ? drupal_render($output) : '';
    }
    return '';
  }
  
  /**
   * Ensure sensible default options for each QuickSet object.
   */
  private static function getDefaultOptions() {
    return array(
      'title' => '<none>',
      'style' => 'nostyle',
      'hide_empty_tabs' => 0,
      'ajax' => 0,
      'default_tab' => 0
    );
  }
  
  /**
   * Constructor
   */
  public function __construct($name, $contents, $options) {
    $this->name = $name;
    $this->contents = array();
    foreach ($contents as $key => $settings) {
      // Instantiate a content renderer object and add it to the contents array.
      if ($renderer = self::getContentRenderer($settings)) {
        $this->contents[$key] = $renderer;
      }
    }
    $default_options = self::getDefaultOptions();
    $this->options = array_merge($default_options, $options);
    $this->prepareContents();
    // Set the default style if necessary.
    if ($this->options['style'] == 'default') $this->options['style'] = variable_get('quicktabs_tabstyle', 'nostyle');
  }

  /**
   * Returns an ajax path to be used on ajax-enabled tab links.
   *
   * @param $index The index of the tab, i.e where it fits into the QuickSet
   * instance.
   *
   * @param $type The type of content we are providing an ajax path for.
   */
  public function getAjaxPath($index, $type) {
    return 'quicktabs/ajax/'. $this->name .'/'. $index . '/'. $type;
  }

  /**
   * This method does some initial set-up of the tab contents, such as hiding
   * tabs with no content if the hide_empty_tabs option is set. It also makes sure
   * that prerendered contents are never attempted to be loaded via ajax.
   *
   * @throws InvalidQuickSetException if there are no contents to render.
   */
  protected function prepareContents() {
    if (!count($this->contents))  {
      throw new InvalidQuickSetException('There are no contents to render.');
    }
    if ($this->options['hide_empty_tabs'] && !$this->options['ajax']) {
      // Check if any tabs need to be hidden because of empty content.
      foreach ($this->contents as $key => $tab) {
        $contents = $tab->render(TRUE);
        if (empty($contents)) {
          unset($this->contents[$key]);
        }
      }
      // Update the array keys in case we removed some tabs.
      $this->contents = array_merge($this->contents);
      if (!count($this->contents))  {
        throw new InvalidQuickSetException('There are no contents to render.');
      }
    }
    elseif ($this->options['ajax']) {
      // Make sure that there is at most 1 prerendered tab and it is the default tab.
      // Prerendered content cannot be rendered via ajax.
      $has_prerendered = FALSE; // keep track of whether we have found a prerendered tab.
      foreach ($this->contents as $key => $tab) {
        $type = $tab->getType();
        if ($type == 'prerendered') {
          if (!$has_prerendered) {
            $has_prerendered = TRUE;
            $this->options['default_tab'] = $key;
            // In the case of a direct link to a different tab, the 'default_tab'
            // will be overridden, so we need to make sure it does not attempt
            // to load a pre-rendered tab via ajax. Turn ajax option off.
            if ($this->getActiveTab() !== $key) {
              $this->options['ajax'] = 0;
            }
          }
          else {
            // We are on a second custom tab and the ajax option is set, we cannot
            // render custom tabs via ajax, so we skip out of the loop, set the
            // ajax option to off, and call the method again.
            $this->options['ajax'] = 0;
            $this->prepareContents();
            return;
          }
        }
      }
    }
  }
  
  /**
   * Returns the active tab for a given Quicktabs instance. This could be coming
   * from the URL or just from the settings for this instance. If neither, it
   * defaults to 0.
   */
  public function getActiveTab() {
    $active_tab = isset($this->options['default_tab']) ? $this->options['default_tab'] : key($this->contents);
    $active_tab = isset($_GET['qt-' . $this->name]) ? $_GET['qt-' . $this->name] : $active_tab;
    $active_tab = (isset($active_tab) && isset($this->contents[$active_tab])) ? $active_tab : 0;
    return $active_tab;
  }
}

/**
 * Abstract base class for QuickSet Renderers.
 *
 * A renderer object contains a reference to a QuickSet object, which it can
 * then render.
 */
abstract class QuickRenderer {
  
  /**
   * @var QuickSet
   */
  protected $quickset;

  /**
   * Constructor
   */
  public function __construct($quickset) {
    $this->quickset = $quickset;
  }
  
  /**
   * Accessor method for the title.
   */
  public function getTitle() {
    return $this->quickset->getTitle();
  }
  
  /**
   * The only method that renderer plugins need to implement.
   * 
   * @return A render array to be passed to drupal_render().
   */
  abstract public function render();
  
}

/*******************************************************
 * The classes below relate to individual tab content  *
 *******************************************************/

/**
 * Each QuickSet object has a "contents" property which is an array of objects
 * that implement the QuickContentRenderable interface.
 */
interface QuickContentRenderable {
  
  /**
   * Returns the short type name of the content plugin, e.g. 'block', 'node', 
   * 'prerendered'.
   */
  public static function getType();
  
  /**
   * Returns the tab title.
   */
  public function getTitle();

  /**
   * Returns an array of settings specific to the type of content.
   */
  public function getSettings();
  
  /**
   * Renders the content.
   *
   * @param $hide_emtpy If set to true, then the renderer should return an empty
   * array if there is no content to display, for example if the user does not
   * have access to the requested content.
   *
   * @param $args Used during an ajax call to pass in the settings necessary to
   * render this type of content.
   */
  public function render($hide_empty = FALSE, $args = array());

  /**
   * Returns an array of keys to use for constructing the correct arguments for
   * an ajax callback to retrieve content of this type. The order of the keys
   * returned affects the order of the args passed in to the render method when
   * called via ajax (see the render() method above).
   */
  public function getAjaxKeys();
  
}

/**
 * Abstract base class for content plugins.
 */
abstract class QuickContent implements QuickContentRenderable {

  /**
   * Used as the title of the tab.
   * @var string
   */
  protected $title;

  /**
   * An array containing the information that defines the tab content, specific
   * to its type.
   * @var array
   */
  protected $settings;
  
  /**
   * A render array of the contents.
   * @var array
   */
  protected $rendered_content;
  

  /**
   * Constructor
   */
  public function __construct($item) {
    $this->title = isset($item['title']) ? $item['title'] : '';
    // We do not need to store title, type or weight in the settings array, which
    // is for type-specific settings.
    unset($item['title'], $item['type'], $item['weight']);
    $this->settings = $item;
  }
  
  
  /**
   * Accessor for the tab title.
   */
  public function getTitle() {
    return $this->title;
  }

  /**
   * Accessor for the tab settings.
   */
  public function getSettings() {
    return $this->settings;
  }

  /**
   * Instantiate a content type object.
   *
   * @param $name
   *   The type name of the plugin.
   *
   * @param $item
   *   An array containing the item definition
   *  
   */
  public static function factory($name, $item) {
    ctools_include('plugins');
    if ($class = ctools_plugin_load_class('quicktabs', 'contents', $name, 'handler')) {
      // We now need to check the plugin's dependencies, to make sure they're installed.
      // This info has already been statically cached at this point so there's no
      // harm in making a call to ctools_get_plugins().
      $plugin = ctools_get_plugins('quicktabs', 'contents', $name);
      if (isset($plugin['dependencies'])) {
        foreach ($plugin['dependencies'] as $dep) {
          // Do not load the options form for any plugin that is missing dependencies.
          if (!module_exists($dep)) return NULL;
        }
      }
      return new $class($item);
    }
    return NULL;
  }

  /**
   * Method for returning the form elements to display for this tab type on
   * the admin form.
   *
   * @param $delta Integer representing this tab's position in the tabs array.
   *
   * @param $qt An object representing the Quicktabs instance that the tabs are
   * being built for.
   */
  abstract public function options_form($delta, $qt);
  
}

/**
 * This class implements the same interface that content plugins do but it is not
 * a content plugin. It is a special class for pre-rendered content which is used
 * when "custom" tabs are added to existing Quicktabs instances in a call to
 * quicktabs_build_quicktabs().
 */
class QuickPreRenderedContent implements QuickContentRenderable {
  
  public static function getType() {
    return 'prerendered';
  }
  
  /**
   * Used as the title of the tab.
   * @var title
   */
  protected $title;
  
  /**
   * A render array of the contents.
   * @var array
   */
  protected $rendered_content;


  /**
   * Constructor
   */  
  public function __construct($item) {
    
    $contents = isset($item['contents']) ? $item['contents'] : array();
    if (!is_array($contents)) {
      $contents = array('#markup' => $contents);
    }
    $this->rendered_content = $contents;
    
    $this->title = isset($item['title']) ? $item['title'] : '';
  }
  
  /**
   * Accessor for the tab title.
   */
  public function getTitle() {
    return $this->title;
  }

  /**
   * Prerendered content doesn't need any extra settings.
   */
  public function getSettings() {
    return array();
  }


  /**
   * The render method simply returns the contents that were passed in and
   * stored during construction.
   */
  public function render($hide_empty = FALSE, $args = array()) {
    return $this->rendered_content;
  }
  
  /**
   * This content cannot be rendered via ajax so we don't return any ajax keys.
   */
  public function getAjaxKeys() {
    return array();
  }

}

/**
 * Create our own exception class.
 */
class InvalidQuickSetException extends Exception {
  
}