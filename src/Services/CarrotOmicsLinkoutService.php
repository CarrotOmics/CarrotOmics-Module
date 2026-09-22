<?php

namespace Drupal\carrotomics\Services;

use Drupal\Core\Link;
use Drupal\Core\StringTranslation\StringTranslationTrait;
use Drupal\Core\Url;
use Drupal\tripal\Services\TripalLogger;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Implements linkouts for blast hit sequences.
 */
class CarrotOmicsLinkoutService {

  use StringTranslationTrait;

  /**
   * The Tripal logger service.
   *
   * @var Drupal\tripal\Services\TripalLogger
   */
  protected TripalLogger $logger;

  /**
   * Constructs the linkout service.
   *
   * @param Drupal\tripal\Services\TripalLogger $logger
   *   The Tripal logger service.
   */
  public function __construct(TripalLogger $logger) {
    $this->logger = $logger;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('tripal.logger'),
    );
  }

  /**
   * Returns a list of supported linkout types for this module.
   *
   * This module's implementation of hook_blast_linkout_info will
   * fetch the information from this function.
   *
   * @return array
   *   The list of linkout types.
   */
  public function getLinkoutTypes(): array {
    $types = [];

    $types['carrotomics_jbrowse1'] = [
      // Human-readable Type name to display to users in the BLAST Database
      // create/edit form.
      'name' => $this->t('CarrotOmics Legacy JBrowse 1'),
      // All linkout types for this module use the same service.
      'service' => 'carrotomics.linkout_service',
      // Help text to show in the BLAST Database create/edit form so that
      // users will know how to use this linkout type. Specifically, info
      // about your assumptions for the URL prefix are very helpful.
      // HTML is allowed but do not enclose in <p>.
      'help' => $this->t('Generate a linkout to the Tripal3 site JBrowse'),
      // Whether or not the linkout requires additional fields from the nodes.
      'require_regex' => TRUE,
      'require_db' => TRUE,
    ];

    return $types;
  }

  /**
   * Implements this modules linkout hook types.
   *
   * Other modules can define other linkout types, definitions are added with
   * hook_blast_linkout_info and the module will define its own instance of
   * this function.
   *
   * @param string $linkout_type
   *   The type of linkout to be generated. This type may be defined
   *   either here or in another module.
   * @param string $url_prefix
   *   The URL prefix for the BLAST Database queried. This originally comes
   *   from the chado.db table urlprefix column.
   * @param string $hit_name
   *   The value that will be displayed in the created link.
   * @param \SimpleXMLElement $hit
   *   The blast XML hit object. This object has the following keys based on the
   *   XML: Hit_num, Hit_id, Hit_def, Hit_accession, Hit_len and Hit_hsps.
   *   Furthermore, a linkout_id key has been added that contains the part of
   *   the Hit_def extracted using a regex provided when the blastdb record was
   *   created.
   * @param array $info
   *   Additional information that may be useful in creating a linkout.
   *   This can include:
   *     - query_name: the name of the query sequence.
   *     - score: the score of the blast hit.
   *     - e-value: the e-value of the blast hit.
   * @param array $options
   *   Any additional options needed to determine the type of linkout.
   *
   * @return \Drupal\Core\Link|string|null
   *   An html link or string if type is supported, or NULL if not.
   */
  public function createLinkout(string $linkout_type, string $url_prefix, string $hit_name, \SimpleXMLElement $hit, array $info, array $options = []): Link|string {
    $link = NULL;
    if ($linkout_type === 'carrotomics_jbrowse1') {
      $link = $this->handleCarrotOmicsJbrowse($url_prefix, $hit_name, $hit, $info, $options);
    }
    return $link;
  }

  /**
   * Handle the 'link' linkout type.
   *
   * @param string $url_prefix
   *   The URL prefix for the BLAST Database queried. This originally comes
   *   from the chado.db table urlprefix column.
   * @param string $hit_name
   *   The value that will be displayed in the created link.
   * @param \SimpleXMLElement $hit
   *   The blast XML hit object. This object has the following keys based on the
   *   XML: Hit_num, Hit_id, Hit_def, Hit_accession, Hit_len and Hit_hsps.
   *   Furthermore, a linkout_id key has beek added that contains the part of
   *   the Hit_def extracted using a regex provided when the blastdb node was
   *   created.
   * @param array $info
   *   Additional information that may be useful in creating a linkout.
   *   This can include:
   *     - query_name: the name of the query sequence.
   *     - score: the score of the blast hit.
   *     - e-value: the e-value of the blast hit.
   * @param array $options
   *   Any additional options needed to determine the type of linkout.
   *   None are used for this linkout type, parameter is here for consistency.
   *
   * @return \Drupal\Core\Link|string
   *   An html link if supported, or a string value of the hit hame if not.
   */
  protected function handleCarrotOmicsJbrowse(string $url_prefix, string $hit_name, \SimpleXMLElement $hit, array $info, array $options = []): Link|string {
    // Fallback if link cannot be created is just the hit name.
    $link = $hit_name;

    if (!$url_prefix || !isset($hit->linkout_id)) {
      return $link;
    }

    // First we need to collect the HSPs to define the ranges we want to
    // display on the JBrowse.
    $ranges = [];
    // We also keep track of all the coordinates in order to later
    // calculate the smallest and largest coordinate.
    $coords = [];
    $count = 0;
    $strands = [];
    foreach ($info['HSPs'] as $hsp) {
      $count++;

      $strand = '1';
      $hsp_start = $hsp['Hsp_hit-from'];
      $hsp_end = $hsp['Hsp_hit-to'];
      $strands[] = $strand;

      // Handle alignments on the negative strand.
      if (($hsp_end - $hsp_start) < 0) {
        $strand = '-1';
        $hsp_start = $hsp['Hsp_hit-to'];
        $hsp_end = $hsp['Hsp_hit-from'];
      }

      // Add both the start & stop to the coordinate list.
      $coords[] = $hsp['Hsp_hit-from'];
      $coords[] = $hsp['Hsp_hit-to'];

      // Format the hsp for inclusion in the subfeatures section of the
      // track later.
      $hsp_def = '{"start":' . $hsp_start . ',"end":' . $hsp_end . ',"strand":"' . $strand . '","type":"match_part"}';
      $ranges[] = $hsp_def;
    }

    // Calculate the minimum & maximum coordinates.
    $min = min($coords);
    $max = max($coords);

    // We also want some white-space on either side of out hit
    // when we show it in the JBrowse. To make this generic,
    // we want our blast hit to take up 2/3 of the screen, thus
    // we have 1/6 per side for white-space.
    $buffer = round(($max - $min) / 6);
    $screen_start = $min - $buffer;
    $screen_end = $max + $buffer;

    // Now we are finally ready to build the URL.
    // First lets set the location of the hit so the JBrowse focuses
    // in on the correct region.
    $jbrowse_query = [];
    $jbrowse_query['loc'] = 'loc=' . $hit->{'linkout_id'} . ':' . $screen_start . '..' . $screen_end;

    $unique_strands = array_unique($strands);
    if (count($unique_strands) === 1) {
      $strand = end($strands);
      // Next we want to add our BLAST hit to the JBrowse.
      $jbrowse_query['addFeatures'] =
        'addFeatures=[{"seq_id":"' . $hit->{'linkout_id'} . '","start":' . $min . ',"end":' . $max
        . ',"name":"' . $info['query_name'] . ' Blast Hit","strand":' . $strand . ',"subfeatures":['
        . implode(',', $ranges) . ']}]';
    }
    else {
      $jbrowse_query['addFeatures'] =
        'addFeatures=[{"seq_id":"' . $hit->{'linkout_id'} . '","start":' . $min . ',"end":' . $max
        . ',"name":"' . $info['query_name'] . ' Blast Hit","subfeatures":['
        . implode(',', $ranges) . ']}]';
    }

    // Then add a track to display our new feature.
    $jbrowse_query['addTracks'] = 'addTracks=[{"label":"blast","key":"BLAST Result","type":"JBrowse/View/Track/CanvasFeatures","store":"url"}]';

    $url_postfix = implode('&', $jbrowse_query);

    $hit_url = $url_prefix . $url_postfix;

    try {
      $url = Url::fromUri($hit_url);
      $url->setOptions([
        'attributes' => [
          'target' => '_blank',
        ],
      ]);
      $link = Link::fromTextAndUrl($hit->linkout_id, $url);
    }
    catch (\Exception $e) {
      // A failed link situation should not be shown to end-users, because
      // they can't do anything about it, but the site admin should see it,
      // so we need to just log this.
      $this->logger->error('CarrotOmicsLinkoutService handleLink error: ' . $e->getMessage());
    }

    return $link;
  }

}
