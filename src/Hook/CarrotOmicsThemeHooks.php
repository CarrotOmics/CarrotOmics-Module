<?php

declare(strict_types=1);

namespace Drupal\carrotomics\Hook;

use Drupal\Core\Hook\Attribute\Hook;

/**
 * Handles asset attachment hooks for CarrotOmics.
 */
class CarrotOmicsThemeHooks {

  /**
   * Implements hook_page_attachments().
   */
  #[Hook('page_attachments')]
  public function pageAttachments(array &$attachments): void {
    $attachments['#attached']['library'][] = 'carrotomics/olivero_carrotomics';
  }

}
