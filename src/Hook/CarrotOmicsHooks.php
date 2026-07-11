<?php

namespace Drupal\carrotomics\Hook;

use Drupal\Core\Hook\Attribute\Hook;
use Drupal\Core\Routing\RouteMatchInterface;
use Drupal\Core\StringTranslation\StringTranslationTrait;
use Drupal\carrotomics\Services\CarrotOmicsRebuildService;

/**
 * Hook implementations for the CarrotOmics module.
 */
class CarrotOmicsHooks {

  use StringTranslationTrait;


  /**
   * The rebuild service for this module.
   *
   * @var \Drupal\carrotomics\Services\CarrotOmicsRebuildService
   */
  protected CarrotOmicsRebuildService $rebuildService;

  /**
   * Constructor.
   *
   * @param \Drupal\carrotomics\Service\RebuildService $rebuildService
   *   The rebuild service for this module.
   */
  public function __construct(CarrotOmicsRebuildService $rebuildService) {
    $this->rebuildService = $rebuildService;
  }

  /**
   * Implements hook_help().
   */
  #[Hook('help')]
  public function help($route_name, RouteMatchInterface $route_match) {
    switch ($route_name) {
      // Main module help for the carrotomics module.
      case 'help.page.carrotomics':
        $output = '<h3>' . $this->t('About') . '</h3>';
        $output .= '<p>' . $this->t('Extended Tripal themes and templates created for CarrotOmics') . '</p>';
        return $output;

      default:
    }
  }

  /**
   * Implements hook_rebuild().
   */
  #[Hook('rebuild')]
  public function rebuild(): string {
    $this->rebuildService->executeRebuild();
    // Return value of the module name is only used for phpunit tests.
    return 'carrotomics';
  }

  /**
   * Implements hook_uninstall().
   *
   * The hook uninstall does not support attributes and must remain procedural.
   */
  public function carrotomicsUninstall() {
print "CPU2 hook uninstall called\n";//;;;
    // Delete the custom tables created by this module.
    $this->rebuildService->dropCustomChadoTables();
  }

}
