<?php

namespace Drupal\carrotomics\Form;

use Drupal\Core\Config\ConfigFactory;
use Drupal\Core\Form\FormStateInterface;
use Drupal\pgsql\Driver\Database\pgsql\Connection;
use Drupal\tripal_chado\Database\ChadoConnection;
use Drupal\tripal\Services\TripalCitationManager;
use Drupal\tripal\Services\TripalEntityLookup;
use Drupal\tripal\Services\TripalLogger;
use Drupal\tripal\TripalBackendPublish\PluginManager\TripalBackendPublishManager;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Provides the user interface for CarrotOmics publication tools.
 */
class CarrotOmicsAdminPub extends CarrotOmicsAdminFormBase {

  /**
   * Drupal configuration service.
   *
   * @var Drupal\Core\Config\ConfigFactory
   */
  protected ConfigFactory $config_factory;

  /**
   * The Tripal Citation generation service.
   *
   * @var Drupal\tripal\Services\TripalCitationManager
   */
  protected TripalCitationManager $citation_manager;

  /**
   * Cache of citation formats indexed by type_id.
   *
   * @var array
   */
  protected array $citation_formats = [];

  /**
   * CarrotOmicsAdminPub class constructor.
   *
   * Prepares injected services.
   */
  public function __construct(
    Connection $drupal_connection,
    ChadoConnection $chado_connection,
    TripalEntityLookup $entity_lookup_manager,
    TripalLogger $logger,
    TripalBackendPublishManager $publish_manager,
    ConfigFactory $config_factory,
    TripalCitationManager $citation_manager,
  ) {
    parent::__construct($drupal_connection, $chado_connection, $entity_lookup_manager, $logger, $publish_manager);
    $this->config_factory = $config_factory;
    $this->citation_manager = $citation_manager;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('database'),
      $container->get('tripal_chado.database'),
      $container->get('tripal.tripal_entity.lookup'),
      $container->get('tripal.logger'),
      $container->get('tripal.backend_publish'),
      $container->get('config.factory'),
      $container->get('tripal.citation'),
    );
  }

  /**
   * Returns a unique string identifying the form.
   *
   * @return string
   *   The unique string identifying the form.
   */
  public function getFormId() {
    return 'carrotomics_admin_pub_form';
  }

  /**
   * {@inheritdoc}
   *
   * Provides the form definition.
   */
  public function buildForm(array $form, FormStateInterface $form_state) {

    // Attach CSS for stying link buttons.
    $form['#attached']['library'][] = 'carrotomics/carrotomics-admin';

    // An element where submit results will go.
    $storage = $form_state->getStorage();
    if (isset($storage['results'])) {
      $form['pub_results'] = [
        '#type'   => 'fieldset',
        '#title' => 'Results',
        '#markup' => '<div class="form-results">' . $storage['results'] . '</div>',
        '#weight' => -100,
      ];
    }

    // Add a 'Update publication Unique Local Identifier' button.
    $form['pub_uli_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'pub_uli_btn',
      '#value'  => 'Update publication Unique Local Identifier',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . "The \"uniquename\" column in the chado.pub table is named"
      . " \"Unique Local Identifier\" in the widget, but is intended"
      . " to hold the citation. There is also a citation property."
      . " Make sure these match."
      . " Additionally, placeholder citations consisting of a single"
      . " character are expanded using chado_pub_create_citation()"
      . "</em></div><br />",
      '#suffix' => "<hr>",
    ];

    // Add a 'Generate publication URLs' button.
    $form['pub_url_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'pub_url_btn',
      '#value'  => 'Generate publication URLs',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . 'This tool will generate a URL for any publication lacking one,'
      . ' if it has a DOI defined. The URL will be constructed by concatenating'
      . ' "https://doi.org/" with the DOI.<br />For example, for the DOI'
      . ' "10.1038/ng.3565" the URL becomes "https://doi.org/10.1038/ng.3565".'
      . '</em></div><br />',
      '#suffix' => "<hr>",
    ];

    // Add a 'Make abstract URLs clickable' button.
    $form['pub_abstract_url_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'pub_abstract_url_btn',
      '#value'  => 'Make abstract URLs clickable',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . 'This tool will ensure that any URLs in a publication abstract'
      . ' is a clickable link by enclosing it in an appropriate href tag'
      . '</em></div><br />',
      '#suffix' => "<hr>",
    ];

    // Add a 'Share pub and pubprop information' button.
    $form['pub_pub_pubprop_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'pub_pubprop_btn',
      '#value'  => 'Share pub and pubprop information',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . 'This tool will make sure that journal names from the'
      . ' chado.pub table in the series_name column are also present in the'
      . ' chado.pubprop table under cv term "Series Name", and vice versa.'
      . ' This allows publication search to function as expected.'
      . ' Similar sharing is done for "Publisher".'
      . '</em></div><br />',
      '#suffix' => "<hr>",
    ];

    // Add a 'Get maximum pub_id' button.
    $form['pub_max_pub_id_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'max_pub_id_btn',
      '#value'  => 'Get maximum pub_id value',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . "This tool returns the highest value of pub_id in the chado.pub table."
      . " This can be used in conjunction with the Find Duplicate Publications"
      . " button below if the value is retrieved prior to loading additional publications."
      . "</em></div><br />",
      '#suffix' => "<hr>",
    ];

    // Add a 'Find Unlinked Errata' button.
    $form['unlinked_pub_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'unlinked_pub_btn',
      '#value'  => 'Find Unlinked Errata',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . 'Find publications that are erratum or correction publications'
      . ' that have not yet been linked to the original publication'
      . ' as a relationship ("tripal_pub" CV "part of" cvterm) and create'
      . ' a link between them in the chado.pub_relationship table'
      . '</em></div><br />',
      '#suffix' => "<hr>",
    ];

    // Add a 'Remove Empty Publication Properties' button.
    $form['empty_prop_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'empty_prop_btn',
      '#value'  => 'Remove Empty Publication Properties',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . 'Remove records in the chado.pubprop table where the value'
      . ' is either NULL or is an empty string'
      . '</em></div><br />',
      '#suffix' => "<hr>",
    ];

    // Add a 'Find Duplicate Publications' button and optional
    // minimum value for pub_id.
    $form['merge_pub_min_pub_id'] = [
      '#type'     => 'textfield',
      '#name'     => 'merge_pub_min_pub_id',
      '#title'    => 'Optional minimum pub_id value',
      '#required' => FALSE,
      '#prefix'   => '<div style="padding-top:30px;"><em>'
      . 'Loading publications from both Pubmed and National Ag Library can'
      . ' result in duplicate instances of the same publication. This tool will'
      . ' generate a list of candidates that can be merged with the Mainlab Chado'
      . ' Loader </em>merge_pub<em> template. Candidate duplications are found with a'
      . ' combination of DOI and Title comparisons. The first column will'
      . ' be the publication to be kept, the second column is the publication'
      . ' that will be deleted. Several additional columns are included for'
      . ' manual examination. Remember to name the Excel tab "merge_pub".'
      . '</em></div><br />',
    ];

    // Add a 'Find Duplicate Publications' button.
    $form['merge_pub_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'merge_pub_btn',
      '#value'  => 'Find Duplicate Publications',
      '#suffix' => "<hr>",
    ];

    // Add a 'Validate publications' button.
    $form['pub_val_btn'] = [
      '#type'   => 'submit',
      '#name'   => 'pub_val_btn',
      '#value'  => 'Validate publications (legacy)',
      '#prefix' => '<div style="padding-top:30px;"><em>'
      . 'This tool will detect problematic publication entries. Publication'
      . ' year must be blank, or be a four digit integer. Non-numeric content'
      . ' causes the chado publication search to fail if a year range is'
      . ' specified, e.g. "Fall 2011" or "06 Sep 2020".'
      // Tripal 3 legacy removed: Publication citation'
      // . ' must contain the full title exactly, otherwise entity links from'
      // . ' the schema__publication field will not work.'.
      . '</em></div><br />',
      '#suffix' => "<hr>",
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {

    $form_state->setRebuild(TRUE);

    // Gets the triggering element, i.e. which button was pressed.
    $triggering_element = $form_state->getTriggeringElement()['#name'];

    // Gets the values from form input fields.
    $min_pub_id = $form_state->getValue('merge_pub_min_pub_id');

    // Take action depending on which button was pressed.
    $nerrors = 0;
    $status = '';
    if ($triggering_element == 'pub_val_btn') {
      [$nerrors, $status] = $this->validatePubs();
    }
    elseif ($triggering_element == 'pub_uli_btn') {
      [$nerrors, $status] = $this->pubUli();
    }
    elseif ($triggering_element == 'pub_url_btn') {
      [$nerrors, $status] = $this->addDoiUrl();
    }
    elseif ($triggering_element == 'pub_abstract_url_btn') {
      [$nerrors, $status] = $this->abstractUrl();
    }
    elseif ($triggering_element == 'pub_pubprop_btn') {
      [$nerrors, $status] = $this->pubPubprop();
    }
    elseif ($triggering_element == 'max_pub_id_btn') {
      [$nerrors, $status] = $this->maxPubId();
    }
    elseif ($triggering_element == 'merge_pub_btn') {
      [$nerrors, $status] = $this->mergePublications($min_pub_id);
      if ($nerrors != -1) {
        $this->returnFile('Duplicate Publications', $form_state);
      }
    }
    elseif ($triggering_element == 'unlinked_pub_btn') {
      [$nerrors, $status] = $this->unlinkedPub();
    }
    elseif ($triggering_element == 'empty_prop_btn') {
      [$nerrors, $status] = $this->dropEmptyProperties();
    }
    else {
      [$nerrors, $status] = [1, "Unknown button \"$triggering_element\" was pressed"];
    }
    $form_state->setStorage(['results' => $status]);
    if ($nerrors > 0) {
      $this->messenger()->addError($nerrors . ' errors');
    }
  }

  /**
   * Validate content for publications.
   *
   * The chado.pub pyear column is either blank, or is a four digit integer.
   * A non-mumeric value here will cause publication search to fail if a
   * year range is specified.
   *
   * This tool does not make any modifications, it only reports problems.
   *
   * @todo this is a tripal 3 legacy, probably no longer needed.
   */
  protected function validatePubs() {
    $errors = '';
    $nerrors = 0;
    $invalidpubs = [];

    // Check every publication for pyear validity.
    $query = $this->chado_connection->select('1:pub', 'P');
    $query->fields('P', ['pub_id', 'pyear']);
    $results = $query->execute();
    foreach ($results as $pub) {
      $pub_id = $pub->pub_id;
      $pyear = $pub->pyear;
      if ($pyear && (!preg_match('/^\d\d\d\d$/', $pyear))) {
        $invalidpubs[] = [$pub_id, 'Invalid pub year', $pyear];
      }
    }

    // Tripal 3 legacy, no longer needed.
    $query = $this->chado_connection->select('1:pub', 'P');
    $query->leftJoin('1:pubprop', 'PP', '[P].[pub_id] = [PP].[pub_id]');
    $query->fields('P', ['pub_id', 'title']);
    $query->fields('PP', ['value']);

    // Subquery 1: Get cv_id from cv table.
    $subquery_cv = $this->chado_connection->select('1:cv', 'C')
      ->fields('C', ['cv_id'])
      ->condition('name', 'tripal_pub', '=');

    // Subquery 2: Get cvterm_id from cvterm table using the first subquery.
    $subquery_cvterm = $this->chado_connection->select('1:cvterm', 'T')
      ->fields('T', ['cvterm_id'])
      ->condition('name', 'Citation')
      ->condition('cv_id', $subquery_cv, '=');

    // Add the WHERE clause using the second subquery.
    $query->condition('PP.type_id', $subquery_cvterm, '=');

    $results = $query->execute();
    foreach ($results as $pub) {
      $pub_id = $pub->pub_id;
      $title = $pub->title;
      $citation = $pub->value;
      if (!$title) {
        $invalidpubs[] = [$pub_id, 'Publication has no title', $citation];
      }
      elseif (!$citation) {
        $invalidpubs[] = [$pub_id, 'Publication has no citation', $title];
      }
      else {
        $pos1 = strpos($citation, $title);
        // Some citations have html for italics but the title does not.
        $citation2 = preg_replace('/<\/?i>/i', '', $citation);
        $pos2 = strpos($citation2, $title);
        $citation3 = preg_replace('/<\/?em>/i', '', $citation);
        $pos3 = strpos($citation3, $title);
        if (($pos2 === FALSE) && ($pos3 === FALSE)) {
          $invalidpubs[] = [$pub_id, 'Citation does not contain exact title', $citation];
        }
        elseif ($pos1 === FALSE) {
          // This happened most commonly for theses.
          $invalidpubs[] = [$pub_id, 'Citation contains italics, title does not', $citation];
        }
      }
    }

    // Display any errors with a link to the corresponding entity.
    if ($invalidpubs) {
      $errors .= '<ol>';
      foreach ($invalidpubs as $pubref) {
        $errors .= '<li>' . $this->entityLink('pub', $pubref[0]) . $pubref[1] . ' "' . $pubref[2] . '"</li>';
        $nerrors++;
      }
      $errors .= '</ol>';
    }

    $status = $this->t('Found @nerrors problems', ['@nerrors' => $nerrors]);
    return [$nerrors, $status . $errors];
  }

  /**
   * If missing, adds a url based on the DOI if possible.
   *
   * For every publication that does not currently have a URL, see if it has
   * a DOI, and if so, construct a URL based on the DOI, i.e.:
   * $url = 'https://doi.org/' . $doi;
   */
  protected function addDoiUrl() {
    $errors = [];

    // Retrieve an array of all existing URL entries, key is pub_id.
    $urls = [];
    $query = $this->chado_connection->select('1:pubprop', 'PP');
    $query->fields('PP', ['pub_id', 'value']);
    $query->condition('PP.type_id', $this->LookupCvterm('URL'), '=');
    $urlresults = $query->execute();
    foreach ($urlresults as $pp) {
      $urls[$pp->pub_id] = $pp->value;
    }

    // Retrieve an array of all existing DOI entries, key is pub_id.
    $dois = [];
    $query = $this->chado_connection->select('1:pubprop', 'PP');
    $query->fields('PP', ['pub_id', 'value']);
    $query->condition('PP.type_id', $this->LookupCvterm('DOI'), '=');
    $doiresults = $query->execute();
    foreach ($doiresults as $pp) {
      $dois[$pp->pub_id] = $pp->value;
    }

    // Loop to add missing URLs. All added URLs will have a rank of zero.
    // Since they are new, there is no possibility of conflict.
    $default_rank = 0;
    $nadded = 0;
    $nerrors = 0;
    foreach ($dois as $pub_id => $doi) {
      // Skip if this pub already has a URL.
      if (!array_key_exists($pub_id, $urls)) {
        // Skip any unusual DOI entries, e.g. OCLC:ocm49858565.
        if (preg_match('/^10\./', $doi)) {
          // Generate a new URL and add it.
          $url = 'https://doi.org/' . $doi;
          $query = $this->chado_connection->insert('1:pubprop');
          $query->fields([
            'pub_id' => $pub_id,
            'type_id' => $this->lookupCvterm('URL'),
            'value' => $url,
            'rank' => $default_rank,
          ]);
          try {
            $pkey = $query->execute();
            if ($pkey) {
              $nadded++;
              $this->logger->notice('addDoiUrl: pub_id=@pub_id inserted new URL property "@url" pubprop_id=@pkey',
                ['@pub_id' => $pub_id, '@url' => $url, '@pkey' => $pkey]);
              $this->needs_republishing['pub'][$pub_id] = TRUE;
            }
            else {
              $nerrors++;
              $this->logger->error('addDoiUrl: pub_id=@pub_id failed to insert new URL property "@url"',
                ['@pub_id' => $pub_id, '@url' => $url]);
            }
          }
          catch (\Exception $e) {
            $nerrors++;
            $errors[] = "addDoiUrl: pub_id=@pub_id exception inserting new URL property \"@url\": " . $e->getMessage();
            $this->logger->error('addDoiUrl: pub_id=@pub_id exception inserting new URL property: @msg',
              ['@pub_id' => $pub_id, '@url' => $url, '@msg' => $e->getMessage()]);
          }
        }
      }
    }

    // This will republish if we indicated any updates.
    $this->republish();

    $errors = $this->flattenErrors($errors);
    $status = $this->t('Added @nadded URLs, @nerrors errors', ['@nadded' => $nadded, '@nerrors' => $nerrors]);
    return [$nerrors, $status . $errors];
  }

  /**
   * Make abstract URLs clickable by adding a "a href" element.
   */
  protected function abstractUrl() {

    $errors = [];
    $nchanged = 0;
    $nerrors = 0;

    // Loop through all retrieved abstracts, process as necessary.
    $query = $this->chado_connection->select('1:pubprop', 'PP');
    $query->fields('PP', ['pubprop_id', 'pub_id', 'value']);
    $query->condition('PP.type_id', $this->lookupCvterm('Abstract'), '=');
    $query->where("LOWER([PP].[value]) LIKE '%http%'");
    $results = $query->execute();
    foreach ($results as $pp) {
      $pubprop_id = $pp->pubprop_id;
      $pub_id = $pp->pub_id;
      $abstracttext = $pp->value;
      // Here come the tricky bits. First make sure it is not already a link
      // by looking for href=. If that is present anywhere in the abstract,
      // then ignore this abstract.
      if ($abstracttext && !preg_match('/href=/i', $abstracttext)) {
        // The tricky part is finding the end of the URL, and often they end
        // in periods that are not part of it, but URLs will contain periods
        // internally.
        // phpcs:ignore
        // Tested with: perl -pe 's/(https?:\/\/[^<\(\)\] \,\n\r]+[^\<\(\)\] \,\.\n\r])/<a href="$1">$1<\/a>/i'.
        $pattern = '/(https?:\/\/[^<\(\)\] \,\n\r]+[^\<\(\)\] \,\.\n\r])/i';
        $replacement = '<a href="${1}">${1}</a>';
        $newabstracttext = preg_replace($pattern, $replacement, $abstracttext);
        if ($newabstracttext != $abstracttext) {
          // Abstract has been updated, so save changes to both
          // drupal and chado.
          $status = $this->microPublish('pub_abstract', $pub_id, $newabstracttext);
          if ($status) {
            $nerrors++;
            $errors[] = $status;
            $this->logger->error('abstractUrl: pub_id=@pub_id failed to micropublish: @status',
              ['@pub_id' => $pub_id, '@status' => $status]);
          }
          else {
            // If that worked, then update chado.
            $query = $this->chado_connection->update('1:pubprop');
            $query->condition('pubprop_id', $pubprop_id, '=');
            $query->fields(['value' => $newabstracttext]);
            $count = $query->execute();
            if ($count) {
              $nchanged += $count;
              $this->logger->notice('abstractUrl: pub_id=@pub_id added href link to abstract',
                ['@pub_id' => $pub_id]);
              $this->needs_republishing['pub'][$pub_id] = TRUE;
            }
            else {
              $nerrors++;
              $errors[] = $this->entityLink('pub', $pub_id) . ' failed to update abstract (count = 0)';
              $this->logger->error('abstractUrl: pub_id=@pub_id failed to update abstract (count = 0)',
                ['@pub_id' => $pub_id]);
            }
          }
        }
      }
    }

    // This will republish if we indicated any updates.
    $this->republish();

    $errors = $this->flattenErrors($errors);
    $status = $this->t('Updated @nchanged abstracts, @nerrors errors',
                ['@nchanged' => $nchanged, '@nerrors' => $nerrors]);
    return [$nerrors, $status . $errors];
  }

  /**
   * Share information between pub and pubprop tables.
   *
   * Publication search checks the pubprop table and ignores the series_name
   * column in the chado.pub table. Make sure the information is in both places.
   * Do likewise for the publisher column.
   */
  protected function pubPubprop() {

    // Retrieve an array of all existing properties of interest.
    $terms = [
      $this->lookupCvterm('Series Name'),
      $this->lookupCvterm('Journal Name'),
      $this->lookupCvterm('Publisher'),
    ];
    $query = $this->chado_connection->select('1:pubprop', 'PP');
    $query->fields('PP', ['pub_id', 'type_id', 'value']);
    $query->condition('PP.type_id', $terms, 'IN');
    $ppresults = $query->execute();
    foreach ($ppresults as $pp) {
      $props[$pp->pub_id][$pp->type_id] = $pp->value;
    }

    // Retrieve an array of all existing pub entries, key is pub_id.
    $pubs = [];
    $query = $this->chado_connection->select('1:pub', 'P');
    $query->fields('P', ['pub_id', 'series_name', 'publisher']);
    $pubresults = $query->execute();
    foreach ($pubresults as $pub) {
      $pub_id = $pub->pub_id;
      $pubs[$pub_id]['series_name'] = $pub->series_name;
      $pubs[$pub_id]['publisher'] = $pub->publisher;
    }

    // Main loop to add missing pub or pubprop values.
    $stats = [
      'nokay' => 0,
      'npropadded' => 0,
      'nadded' => 0,
      'nerrors' => 0,
      'nnull' => 0,
      'errortext' => [],
    ];
    foreach ($pubs as $pub_id => $pub_values) {
      $this->pubPubpropHelper($stats, $props, $pub_id, $pub_values, 'series_name', $this->lookupCvterm('Series Name'), TRUE);
      $this->pubPubpropHelper($stats, $props, $pub_id, $pub_values, 'publisher', $this->lookupCvterm('Publisher'), FALSE);
    }

    // This will republish if we indicated any updates.
    $this->republish();

    $status = $this->t('@nokay value checks were okay, added @npropadded values to chado.pubprop, added @nadded values to chado.pub, @nerrors errors',
      [
        '@nokay' => $stats['nokay'],
        '@npropadded' => $stats['npropadded'],
        '@nadded' => $stats['nadded'],
        '@nerrors' => $stats['nerrors'],
      ]);
    $errors = $this->flattenErrors($stats['errortext']);
    return [$stats['nerrors'], $status . $errors];
  }

  /**
   * Helper function for pubPubprop().
   *
   * @return void
   *   The status result is returned by updating the $stats array.
   */
  protected function pubPubpropHelper(&$stats, $props, $pub_id, $pub_values, $column, $type_id, $try_journal = FALSE): void {

    // All added records will have this default rank value. Since
    // they are new, there is no possibility of conflict.
    $default_rank = 0;

    // The value from the chado.pub table.
    $property_value = $pub_values[$column] ?? NULL;

    // Select different logic if column in pub table is or isn't populated.
    if ($property_value) {
      // A value is present in pub table, check if present in pubprop table.
      if ((array_key_exists($pub_id, $props)) && (array_key_exists($type_id, $props[$pub_id]))) {
        // Present in both places, now check if they are be the same.
        if ($property_value != $props[$pub_id][$type_id]) {
          // If different, these cases will have to be fixed manually.
          $stats['errortext'][] = $this->entityLink('pub', $pub_id) . "Discrepancy"
                              . " $column=\"$property_value\" property=\"" . $props[$pub_id][$type_id] . "\"";
          $stats['nerrors']++;
          $this->logger->warning('pubPubProp: pub_id=@pub_id discrepancy column "@col" value "@val" != property "@prop"',
            ['@pub_id' => $pub_id, '@col' => $column, '@val' => $property_value, '@prop' => $props[$pub_id][$type_id]]);
        }
        else {
          $stats['nokay']++;
        }
      }
      // A value is in chado.pub table, but a pubprop is not present, so
      // property should be added using the value from the chado.pub table.
      else {
        $query = $this->chado_connection->insert('1:pubprop');
        $query->fields([
          'pub_id' => $pub_id,
          'type_id' => $type_id,
          'value' => $property_value,
          'rank' => $default_rank,
        ]);
        try {
          $result = $query->execute();
          if ($result) {
            $stats['npropadded']++;
            $this->logger->notice('pubPubProp: pub_id=@pub_id inserted new property type "@type" value "@value"',
              ['@pub_id' => $pub_id, '@type' => $column, '@value' => $property_value]);
            $this->needs_republishing['pub'][$pub_id] = TRUE;
          }
          else {
            $stats['nerrors']++;
            $stats['errortext'][] = 'Error inserting property for pub_id $pub_id';
            $this->logger->error('pubPubProp: pub_id=@pub_id error inserting new property type "@type" value "@value"',
              ['@pub_id' => $pub_id, '@type' => $column, '@value' => $property_value]);
          }
        }
        catch (\Exception $e) {
          $stats['nerrors']++;
          $stats['errortext'][] = 'Exception inserting property: ' . $e->getMessage();
          $this->logger->error('pubPubProp: pub_id=@pub_id exception inserting new property type "@type" value "@value": @msg',
            ['@pub_id' => $pub_id, '@type' => $column, '@value' => $property_value, '@msg' => $e->getMessage()]);
        }
      }
    }
    // A value is not present in the chado.pub table, check whether we can
    // copy from pubprop to pub table, with an optional special case for
    // "Journal Name".
    else {
      if (array_key_exists($pub_id, $props)) {
        $property_value = '';
        if (array_key_exists($type_id, $props[$pub_id])) {
          $property_value = $props[$pub_id][$type_id];
        }
        // This only applies to series name, where journal can be used
        // as a substitute.
        elseif ($try_journal) {
          $journal_type_id = $this->lookupCvterm('Journal Name');
          if (array_key_exists($journal_type_id, $props[$pub_id])) {
            $property_value = $props[$pub_id][$journal_type_id];
          }
          else {
            $stats['nnull']++;
          }
        }
        else {
          $stats['nnull']++;
        }
        if ($property_value) {
          try {
            $query = $this->chado_connection->update('1:pub');
            $query->condition('pub_id', $pub_id, '=');
            $query->fields([$column => $property_value]);
            $count = $query->execute();
            if ($count) {
              $stats['nadded']++;
              $this->logger->notice('pubPubProp: pub_id=@pub_id copied property "@value" to pub table column "@column"',
                ['@pub_id' => $pub_id, '@value' => $property_value, '@column' => $column]);
              $this->needs_republishing['pub'][$pub_id] = TRUE;
            }
            else {
              $stats['nerror']++;
              $this->logger->error('pubPubProp: pub_id=@pub_id error copying property "@value" to pub table column "@column"',
                ['@pub_id' => $pub_id, '@value' => $property_value, '@column' => $column]);
            }
          }
          catch (\Exception $e) {
            $stats['nerror']++;
            $stats['errortext'][] = 'Exception copying property to pub: ' . $e->getMessage();
            $this->logger->error('pubPubProp: pub_id=@pub_id exception copying property "@value" to pub table column "@column: @msg"',
              ['@pub_id' => $pub_id, '@value' => $property_value, '@column' => $column, '@msg' => $e->getMessage()]);
          }
        }
      }
      else {
        $stats['nnull']++;
      }
    }
  }

  /**
   * Retrieve the highest pub_id value from the chado.pub table.
   */
  protected function maxPubId() {
    $query = $this->chado_connection->select('1:pub', 'P');
    $query->addExpression('MAX([P].[pub_id])', 'max_pub_id');
    $max_pub_id = $query->execute()->fetchField();

    if ($max_pub_id) {
      return [0, "Maximum pub_id = $max_pub_id"];
    }
    else {
      return [1, 'Error retrieving maximum pub_id value'];
    }
  }

  /**
   * Generate a file to merge duplicate publications.
   *
   * This tool does not make any modifications, it only reports problems.
   */
  protected function mergePublications($min_pub_id = NULL) {

    // The first three columns are for the Mainlab Chado Loader. The other
    // columns are information for the manual filtering process.
    $headers = [
      '*pub_id_base',
      '*pub_id',
      'delete',
      '(entity_base)',
      '(entity)',
      '(property)',
      '(duplicated value)',
      '(year_base)',
      '(year)',
      '(volume_base)',
      '(volume)',
      '(issue_base)',
      '(issue)',
      '(page_base)',
      '(page)',
      '(journal_base)',
      '(journal)',
      '(title_base)',
      '(title)',
    ];
    // Characters to remove to make a title hash key. Sometimes one
    // version of a title ends in a period, other times it doesn't.
    // Dashes and ndashes may be subsituted for each other.
    // Remove any HTML tags.
    // Quotes single or double can vary, remove them.
    $titlehashpattern = [
      '/\.+/',
      '/\s+/',
      '/\-+/',
      '/‐+/',
      '/—+/',
      '/<[^>]+>/',
      '/‘/',
      '/’/',
      "/'/",
      '/“/',
      '/”/',
      '/"/',
    ];

    // This will store key=>lower_pub_id value=>[higher_pub_id, info].
    $candidates = [];
    // Cache of other information from each pub, key is pub_id.
    $pub_info = [];

    // Duplicate detection part 1 = based on specific properties.
    // These cvterms will be used to find duplicates, based on just that
    // single value. Consider https:// and http:// to be equivalent.
    $terms = [
      'DOI' => $this->lookupCvterm('DOI'),
      'Elocation' => $this->lookupCvterm('Elocation'),
      'URL' => $this->lookupCvterm('URL'),
    ];
    foreach ($terms as $cvtermname => $cvterm_id) {
      $sql = "SELECT A.pub_id, regexp_replace(lower(A.value), '^http:', 'https:') AS filtervalue, A.rank,"
           . " P.title, P.series_name, P.pyear, P.volume, P.issue, P.pages"
           . " FROM {1:pubprop} A"
           . " JOIN (SELECT regexp_replace(lower(value), '^http:', 'https:') AS groupvalue,"
           . "   count(*) FROM {1:pubprop} PP"
           . " WHERE PP.type_id=" . $cvterm_id
           . " GROUP BY groupvalue HAVING COUNT(*) > 1) B"
           . " ON regexp_replace(lower(A.value), '^http:', 'https:') = B.groupvalue"
           . " LEFT JOIN {1:pub} P ON A.pub_id=P.pub_id"
           . " ORDER BY filtervalue, A.pub_id";
      $args = [];
      try {
        $results = $this->chado_connection->query($sql, $args);
      }
      catch (\Exception $e) {
        return [1, $e->getMessage()];
      }

      // Based on the SQL there will be at least two duplicates of a given
      // value, but there may be more. For each set of duplicates, we will
      // store to the candidates hash, with the lowest pub_id as the key.
      // The SQL is sorted, so the first one will always be the lowest pub_id.
      $prevvalue = '';
      $first_pub_id = '';
      foreach ($results as $obj) {
        $pub_id = $obj->pub_id;
        $value = $obj->filtervalue;

        if ($value != $prevvalue) {
          // First of a new set of duplicates, just save the pub_id.
          $first_pub_id = $pub_id;
          $prevvalue = $value;
        }
        else {
          // Second or later of a set of duplicates.
          $candidates[$first_pub_id][] = [$pub_id, $cvtermname, $value];
        }
      }
    }

    // Duplicate detection part 2 = based on title.
    // Try to find duplicates based just on nearly identical titles combined
    // with starting page number.
    $sql = "SELECT P.pub_id, P.title, P.series_name, P.pyear, P.volume, P.issue, P.pages"
         . " FROM {1:pub} P"
         . " ORDER BY P.pub_id";
    $args = [];
    try {
      $results = $this->chado_connection->query($sql, $args);
    }
    catch (\Exception $e) {
      return [1, $e->getMessage()];
    }

    // Load every publication and store by processed title.
    $titlehash = [];
    foreach ($results as $obj) {
      $pub_id = $obj->pub_id;

      // Cache other information from pub for the output file.
      $pub_info[$pub_id]['year'] = $obj->pyear;
      $pub_info[$pub_id]['volume'] = $obj->volume;
      $pub_info[$pub_id]['issue'] = $obj->issue;
      $pub_info[$pub_id]['firstpage'] = preg_replace('/\-.*/', '', ($obj->pages ?? ''));
      $pub_info[$pub_id]['journal'] = $obj->series_name;
      $pub_info[$pub_id]['title'] = $obj->title;

      $title = preg_replace($titlehashpattern, [], strtolower($obj->title));
      if ($title) {
        // No longer include page here, some duplicates are missing page number.
        $key = $title;
        $titlehash[$key][] = $pub_id;
      }
    }

    // Now look for titlehash keys with more than one value.
    foreach ($titlehash as $key => $arrayref) {
      if (count($arrayref) > 1) {
        $pub_ids = $arrayref;
        // Get the lowest pub_id as the first element.
        sort($pub_ids);
        for ($i = 1; $i < count($pub_ids); $i++) {
          $candidates[$pub_ids[0]][] = [$pub_ids[$i], 'matching title', ''];
        }
      }
    }

    // Initialize the output file content.
    $content = implode("\t", $headers) . "\n";
    $nfound = 0;

    // Populate output file.
    ksort($candidates);
    foreach ($candidates as $pub_id => $arrayref) {
      foreach ($arrayref as $idandvalue) {
        $dup_pub_id = $idandvalue[0];
        $dup_pub_cvterm = $idandvalue[1];
        $dup_pub_value = $idandvalue[2];
        $entity = $this->entity_lookup_manager->getEntityId($pub_id, NULL, NULL, 'pub');
        $dup_entity = $this->entity_lookup_manager->getEntityId($dup_pub_id, NULL, NULL, 'pub');
        $cols = [
          $pub_id,
          $dup_pub_id,
          'yes',
          $entity,
          $dup_entity,
          $dup_pub_cvterm,
          $dup_pub_value,
        ];
        // Additional informational columns.
        $coltypes = ['year', 'volume', 'issue', 'firstpage', 'journal', 'title'];
        $flip_base_votes = 0;
        foreach ($coltypes as $coltype) {
          $left = $pub_info[$pub_id][$coltype];
          $right = $pub_info[$dup_pub_id][$coltype];
          if ($coltype == 'title') {
            // For title, case insensitive, and ignore white space and periods.
            if (preg_replace($titlehashpattern, [], strtolower($left)) == preg_replace($titlehashpattern, [], strtolower($right))) {
              $right = '=';
            }
          }
          else {
            // If one publication has volume, page, etc and the other doesn't,
            // then that one should become the base_id. Vote on it.
            if (($left) and (!$right)) {
              // Negative = don't flip.
              $flip_base_votes--;
            }
            elseif ((!$left) and ($right)) {
              // Positive = do flip.
              $flip_base_votes++;
            }
            // If equal, abbreviate the second copy.
            elseif (($left) and ($right) and ($left == $right)) {
              $right = '=';
            }
          }
          $cols[] = $left;
          $cols[] = $right;
        }
        // Check votes, and if positive swap (flip) pub_id_base and pub_id.
        if ($flip_base_votes > 0) {
          $tmp = $cols[0];
          $cols[0] = $cols[1];
          $cols[1] = $tmp;
        }
        // Filtering for optional minimum for pub_id.
        if ((!$min_pub_id) or ($pub_id >= $min_pub_id) or ($dup_pub_id >= $min_pub_id)) {
          // Ignore if match key is URL and it is a generic URL.
          if ($dup_pub_cvterm != 'URL' || $this->validUrl($dup_pub_value)) {
            $content .= implode("\t", $cols) . "\n";
            $nfound++;
          }
        }
      }
    }

    // Save the results to a file.
    file_put_contents($this->result_file_path, $content);

    $this->logger->notice('mergePublications: @nfound publications may need to be merged',
      ['@nfound' => $nfound]);
    // When we return a file to download as the response, no message will be
    // displayed. Returning this message as a negative error will indicate to
    // not return the empty file.
    if (!$nfound) {
      return [-1, "There were no publications found that are candidates for merging"];
    }
  }

  /**
   * Determine if a URL for a publication is not a "general" journal URL.
   *
   * General URLs only link to the journal rather than directly to the
   * publication. Returns TRUE if the URL is likely to point to a publication.
   *
   * @param string $url
   *   The URL to evaluate.
   *
   * @return bool
   *   TRUE if likely links to a publication, FALSE if general journal link.
   */
  protected function validUrl(string $url): bool {
    $result = TRUE;
    // Ignore if URL is just a site and no further path
    // e.g. "http://www.jhortscib.com/".
    if (preg_match('|https?://[^/]+/$|i', $url)) {
      $result = FALSE;
    }
    // Ignore if URL ends in "/contents"
    // e.g. "http://www.kluweronline.com/issn/0032-079X/contents".
    elseif (preg_match('|/contents$|i', $url)) {
      $result = FALSE;
    }
    return $result;
  }

  /**
   * Find errata and corrections that should be linked to the original pub.
   */
  protected function unlinkedPub() {
    $outputmessages = [];
    $nadded = 0;
    $nexisting = 0;
    $nwarnings = 0;
    $nerrors = 0;

    // Determine the cvterm_id for tripal_pub::part of
    // It is potentially possible to also use relationship::part_of, but so
    // far CarrotOmics isn't using that term for this purpose.
    $part_of_id = $this->lookupCvterm('part of');

    // Patterns to be removed from the title to derive the original title:
    // Correction: Blah blah carrot
    // Publisher Correction: Blah blah roots
    // Correction to "Blah blah orange"
    // Erratum: Blah blah apiaceae
    // Erratum to: Blah blah daucus
    // Erratum.
    // Blah blah consumption [Erratum: 2007 Nov., v. 46, issue 2, p. 199.].
    $titlehashpattern = [
      '/^erratum to: */i',
      '/^erratum[:\.] */i',
      '/^correction to: */i',
      '/^correction[:\.] */i',
      '/^publisher correction[:\.] */i',
      '/^corrigendum[:\.] */i',
      '/ *\[erratum[^\]]+\] */i',
      '/retracted article[:\.] */i',
      '/retraction notice[:\.] */i',
      '/retraction note[:\.] */i',
      '/retraction[:\.] */i',
      '/retracted[:\.] */i',
    ];

    // Main query for candidate erratum publications.
    $query1 = $this->chado_connection->select('1:pub', 'P');
    $query1->fields('P', ['pub_id', 'title']);
    $or_group1 = $query1->orConditionGroup();
    $or_group1->condition('P.title', '%erratum%', 'ILIKE');
    $or_group1->condition('P.title', '%correction%', 'ILIKE');
    $or_group1->condition('P.title', '%corrigendum%', 'ILIKE');
    $or_group1->condition('P.title', '%retract%', 'ILIKE');
    $query1->condition($or_group1);
    $results1 = $query1->execute();

    // Loop and investigate each retrieved publication.
    foreach ($results1 as $obj1) {
      $pub_id = $obj1->pub_id;
      $title = $obj1->title;
      $title_lc = strtolower($title);

      // Before proceeding, see if a relationship already exists. This may
      // have been done manually for some of the publications with errors
      // or misspellings in the title or other anomalies, or was done in a
      // previous call to this function.
      $query2 = $this->chado_connection->select('1:pub_relationship', 'R');
      $query2->fields('R', ['pub_relationship_id', 'object_id']);
      $query2->condition('R.subject_id', $pub_id, '=');
      $query2->condition('R.type_id', $part_of_id, '=');
      $results2 = $query2->execute();

      $hits = [];
      foreach ($results2 as $obj2) {
        $hits[] = $obj2->object_id;
      }
      if (count($hits) > 0) {
        $nexisting++;
      }
      else {
        // Here we try to extract the original title.
        $origtitle = preg_replace($titlehashpattern, '', $title_lc);
        // Special handling for quoted titles.
        $origtitle = preg_replace(['/^correction to "(.*?)"(\.?)$/'], '$1$2', $origtitle);
        $origtitle = preg_replace(['/^corrigendum to "(.*?)"(\.?)$/'], '$1$2', $origtitle);

        // If this processing did nothing, it might be due to the word
        // "correction" elsewhere the title. These cases we return as warnings.
        if ($title_lc == $origtitle) {
          $nwarnings++;
          $outputmessages[] = $this->entityLink('pub', $pub_id) . 'Warning: Could not extract an original title'
                         . ', it may not be an actual erratum or correction:<br>' . $origtitle;
        }
        else {
          // Try to retrieve the pub_id of the original publication,
          // this is a case-insensitive query.
          $query3 = $this->chado_connection->select('1:pub', 'P');
          $query3->fields('P', ['pub_id']);
          $query3->condition('P.title', $origtitle, 'ILIKE');
          $results3 = $query3->execute();

          // We want exactly one match. Zero means we couldn't find it,
          // two or more if we have ambiguous matches.
          $orig_pub_id = [];
          foreach ($results3 as $obj3) {
            $orig_pub_id[] = $obj3->pub_id;
          }
          $nmatches = count($orig_pub_id);
          if ($nmatches == 0) {
            $nerrors++;
            $outputmessages[] = $this->entityLink('pub', $pub_id) . "Error: No match to extracted original title from \""
                         . $title
                         . "\" = \"$origtitle\""
                         . " - Perhaps the original publication needs to be added first.";
          }
          elseif ($nmatches > 1) {
            $nerrors++;
            $outputmessages[] = "Error: $nmatches ambiguous matches to extracted original title \"$origtitle\" from \""
                         . $title
                         . "\".";
          }
          else {
            // Here we now have one erratum pub and one original pub to link
            // it to. We know that a relationship does not already exist,
            // because of the query in query2, so we can proceed to add the
            // relationship now.
            $query4 = $this->chado_connection->insert('1:pub_relationship');
            $query4->fields([
              'subject_id' => $pub_id,
              'object_id' => $orig_pub_id[0],
              'type_id' => $part_of_id,
            ]);
            try {
              $result = $query4->execute();
              if ($result) {
                $nadded++;
                $outputmessages[] = 'Linked publication ' . $this->entityLink('pub', $pub_id)
                                . $this->entityLink('pub', $orig_pub_id[0], FALSE);
                $this->needs_republishing['pub'][$pub_id] = TRUE;
                $this->needs_republishing['pub'][$orig_pub_id[0]] = TRUE;
              }
              else {
                $nerrors++;
                $outputmessages[] = 'Failure linking ' . $this->entityLink('pub', $pub_id)
                                . $this->entityLink('pub', $orig_pub_id[0], FALSE);
              }
            }
            catch (\Exception $e) {
              $nerrors++;
              $outputmessages[] = 'Exception linking ' . $this->entityLink('pub', $pub_id)
                                . $this->entityLink('pub', $orig_pub_id[0], FALSE) . ': ' . $e->getMessage();
            }
          }
        }
      }
    }

    // This will republish if we indicated any updates.
    $this->republish();

    if ($outputmessages) {
      return [
        $nerrors,
        "$nexisting existing relationships present, $nadded relationships added, $nerrors errors, $nwarnings warnings"
        . $this->flattenErrors($outputmessages),
      ];
    }
    else {
      return [0, "No unlinked errata found"];
    }
  }

  /**
   * Change the Unique Local Identifier to be the citation.
   */
  protected function pubUli() {
    $errors = [];
    $nerrors = 0;
    $nchanged = 0;
    $ninserted = 0;

    $citation_id = $this->lookupCvterm('Citation');

    // Add a citation property if one is missing.
    $subquery1 = $this->chado_connection->select('1:pubprop', 'PP');
    $subquery1->condition('PP.type_id', $citation_id, '=');
    $subquery1->addExpression('1');
    $subquery1->where('[PP].[pub_id] = [P].[pub_id]');

    $query1 = $this->chado_connection->select('1:pub', 'P');
    $query1->fields('P', ['pub_id']);
    $query1->notExists($subquery1);
    // This is a text string null, not an actual NULL.
    $query1->condition('P.uniquename', 'null', '<>');
    $results1 = $query1->execute();
    foreach ($results1 as $pub) {
      $citation = $this->chadoPubCreateCitation($pub->pub_id);
      if (strlen($citation) > 2) {
        $this->logger->notice('pubUli: pub_id=@pub_id inserting new citation property, citation="@cit"',
          ['@pub_id' => $pub->pub_id, '@cit' => $citation]);
        $query2 = $this->chado_connection->insert('1:pubprop');
        $query2->fields([
          'pub_id' => $pub->pub_id,
          'type_id' => $citation_id,
          'value' => $citation,
          'rank' => 0,
        ]);
        $pubprop_id = $query2->execute();
        if ($pubprop_id) {
          $ninserted++;
          $this->needs_republishing['pub'][$pub->pub_id] = TRUE;
        }
        else {
          $nerrors++;
          $this->logger->error('pubUli: pub_id=@pub_id could not insert citation property, citation="@cit"',
            ['@pub_id' => $pub->pub_id, '@cit' => $citation]);
          $errors[] = $this->entityLink('pub', $pub->pub_id) . 'Could not insert citation property';
        }
      }
      else {
        $nerrors++;
        $errors[] = $this->entityLink('pub', $pub->pub_id) . 'Could not generate a citation';
        $this->logger->error('pubUli: pub_id=@pub_id could not generate a citation',
          ['@pub_id' => $pub->pub_id]);
      }
    }

    // Check for single-character or null citations.
    // A single character during entry through the UI is a special placeholder
    // so that this tool can generate the actual value computationally.
    $query1 = $this->chado_connection->select('1:pub', 'P');
    $query1->leftJoin('1:pubprop', 'PP', '[P].[pub_id] = [PP].[pub_id]');
    $query1->fields('P', ['pub_id']);
    $query1->fields('PP', ['pubprop_id', 'value']);
    $query1->condition('PP.type_id', $citation_id, '=');
    $query1->where('LENGTH([PP].[value]) <= 1');
    $results1 = $query1->execute();

    // Loop and make changes, keeping track of the number of changes made.
    $ncited = 0;
    foreach ($results1 as $pub) {
      $pub_id = $pub->pub_id;
      $pubprop_id = $pub->pubprop_id;
      $citation = $this->chadoPubCreateCitation($pub_id);
      if (strlen($citation) > 2) {
        $this->logger->notice('pubUli: pub_id=@pub_id updating citation property for placeholder "@ph", new citation="@cit"',
          ['@pub_id' => $pub_id, '@ph' => $pub->value, '@cit' => $citation]);
        $query = $this->chado_connection->update('1:pubprop');
        $query->condition('pubprop_id', $pubprop_id, '=');
        $query->fields(['value' => $citation]);
        try {
          $query->execute();
          $ncited++;
        }
        catch (\Exception $e) {
          $nerrors++;
          $errors[] = $this->entityLink('pub', $pub_id) . 'Exception updating citation property: ' . $e->getMessage();
          $this->logger->error('pubUli: pub_id=@pub_id exception updating citation property: @msg',
            ['@pub_id' => $pub->pub_id, '@msg' => $e->getMessage()]);
        }
      }
      else {
        $errors[] = $this->entityLink('pub', $pub_id) . 'No citation could be generated';
        $nerrors++;
        $this->logger->error('pubUli: pub_id=@pub_id could not generate a citation',
          ['@pub_id' => $pub->pub_id]);
      }
    }

    // Find any uniquenames needing to be updated, but leave the special
    // null pub alone. Copy the citation property to the uniquename column
    // if they differ.
    $query2 = $this->chado_connection->select('1:pub', 'P');
    $query2->leftJoin('1:pubprop', 'PP', '[P].[pub_id] = [PP].[pub_id]');
    $query2->fields('P', ['pub_id', 'uniquename']);
    $query2->fields('PP', ['value']);
    $query2->condition('PP.type_id', $citation_id, '=');
    $query2->where('NOT [P].[uniquename] = [PP].[value]');
    // This is a text string null, not an actual NULL.
    $query2->condition('P.uniquename', 'null', '<>');
    $results2 = $query2->execute();

    // Loop and copy the citation property to uniquename, keeping track of
    // the number of changes made.
    // n.b. To prevent HTML <P> tags, the citation widget has been changed
    // from default text filter format of HTML to plain text.
    foreach ($results2 as $pub) {
      try {
        $status = $this->microPublish('pub_uniquename', $pub->pub_id, $pub->value);
        if ($status) {
          $nerrors++;
          $errors[] = $this->entityLink('pub', $pub_id) . 'Error updating citation: ' . $status;
          $this->logger->error('pubUli: pub_id=@pub_id Error updating citation',
            ['@pub_id' => $pub->pub_id]);
        }
        else {
          $query3 = $this->chado_connection->update('1:pub');
          $query3->condition('pub_id', $pub->pub_id, '=');
          $query3->fields(['uniquename' => $pub->value]);
          $success = $query3->execute();
          if ($success) {
            $nchanged++;
            $this->logger->notice('pubUli: pub_id=@pub_id updated uniquename to @name',
              ['@pub_id' => $pub->pub_id, '@name' => $pub->value]);
          }
          else {
            $nerrors++;
            $errors[] = $this->entityLink('pub', $pub->pub_id) . 'Error updating uniquename' . $msg;
            $this->logger->error('pubUli: pub_id=@pub_id Error updating uniquename: @msg',
              ['@pub_id' => $pub->pub_id, '@msg' => $msg]);
          }
        }
      }
      catch (\Exception $e) {
        $nerrors++;
        $errors[] = $this->entityLink('pub', $pub->pub_id) . 'Exception: ' . $e->getMessage();
        $this->logger->error('pubUli: pub_id=@pub_id Exception: @msg',
          ['@pub_id' => $pub->pub_id, '@msg' => $e->getMessage()]);
      }
    }

    // This will republish if we indicated any updates.
    $this->republish();

    // Flatten errors array to string.
    $errors = $this->flattenErrors($errors);

    if ($nchanged or $ncited or $nerrors or $ninserted) {
      $status = $this->t('Changed @nchanged uniquename values, updated @ncited citations, inserted @ninserted citation properties, @nerrors errors',
        ['@nchanged' => $nchanged, '@ncited' => $ncited, '@ninserted' => $ninserted, '@nerrors' => $nerrors]);
    }
    else {
      $status = $this->t('No changes were needed');
    }
    return [$nerrors, $status . $errors];
  }

  /**
   * Remove empty records in the chado.pubprop table.
   *
   * Empty records have either a NULL or an empty string in the value column.
   */
  protected function dropEmptyProperties() {
    $errors = '';
    $nerrors = 0;
    $ndropped = [];

    // This entire function could all be done in one simple SQL
    // statement, but I want statistics of what was dropped.
    // First find the empty properties, then loop through each of them.
    $query1 = $this->chado_connection->select('1:pubprop', 'PP');
    $query1->leftJoin('1:cvterm', 'CV', '[PP].[type_id] = [CV].[cvterm_id]');
    $query1->fields('PP', ['pubprop_id', 'pub_id', 'type_id', 'value', 'rank']);
    $query1->fields('CV', ['name']);
    $query1->where("([PP].[value] = '') IS NOT FALSE OR [PP].[value] = '|||'");
    // '|||' is found in "Published Location".
    $results1 = $query1->execute();

    // Loop and make changes, keeping track of the number of changes
    // made for each cv term.
    foreach ($results1 as $obj) {
      $pubprop_id = $obj->pubprop_id;
      $cvtermname = $obj->name;
      try {
        $query2 = $this->chado_connection->delete('1:pubprop');
        $query2->condition('pubprop_id', $pubprop_id, '=');
        $success = $query2->execute();
        if ($success) {
          $this->logger->notice('dropEmptyProperties: Dropped pub_id=@pub_id pubprop_id=@pp_id term=@term',
            ['@pub_id' => $obj->pub_id, '@pp_id' => $obj->pubprop_id, '@term' => $obj->name]);
          $ndropped[$cvtermname] = ($ndropped[$cvtermname] ?? 0) + 1;
          $this->needs_republishing['pub'][$obj->pub_id] = TRUE;
        }
        else {
          $nerrors++;
          $this->logger->error('dropEmptyProperties: Error pub_id=@pub_id dropping term=@term',
            ['@pub_id' => $pub->pub_id, '@term' => $obj->name]);
        }
      }
      catch (\Exception $e) {
        $nerrors++;
        $this->logger->error('dropEmptyProperties: Exception pub_id=@pub_id dropping term=@term msg=@msg',
          ['@pub_id' => $pub->pub_id, '@term' => $obj->name, '@msg' => $e->getMessage()]);
        $errors = '<br>' . $e->getMessage();
      }
    }

    // This will republish if we indicated any updates.
    $this->republish();

    if ($ndropped or $nerrors) {
      $status = '';
      foreach ($ndropped as $cvtermname => $count) {
        $this->logger->notice('dropEmptyProperties: Dropped @count empty pubprop records for the term "@term"',
          ['@count' => $count, '@term' => $cvtermname]);
        $status .= "Dropped $count empty pubprop records for the term \"$cvtermname\"<br>";
      }
      $status .= "$nerrors Errors";
    }
    else {
      $this->logger->notice('dropEmptyProperties: No changes were needed');
      $status = $this->t('No changes were needed');
    }
    return [$nerrors, $status . $errors];
  }

  /**
   * Creates a new citation for a publication.
   *
   * @param int $pub_id
   *   The value of the pkey in the chado pub table.
   *
   * @return string
   *   The citation.
   */
  protected function chadoPubCreateCitation(int $pub_id): string {
    $properties = [];
    $pub_type = NULL;
    $citation = '';

    // Columns for the pub table. Leave out pub_id, type_id, and is_obsolete.
    $pub_cols = [
      'title',
      'volumetitle',
      'volume',
      'series_name',
      'issue',
      'pyear',
      'pages',
      'miniref',
      'uniquename',
      'publisher',
      'pubplace',
    ];

    // Retrieve values for columns in the pub table.
    $query1 = $this->chado_connection->select('1:pub', 'P');
    $query1->leftJoin('1:cvterm', 'T', '[P].[type_id] = [T].[cvterm_id]');
    $query1->fields('P');
    $query1->fields('T', ['name']);
    $query1->condition('P.pub_id', $pub_id, '=');
    $results1 = $query1->execute();
    // Although a loop, this will only execute zero or one times.
    foreach ($results1 as $pub) {
      foreach ($pub_cols as $pub_col) {
        $properties[$pub_col] = $pub->$pub_col;
      }
      // Store publication type needed for the citation manager service.
      $pub_type = $pub->type_id;
    }

    if ($pub_type) {
      // Retrieve all property values for this publication.
      $query2 = $this->chado_connection->select('1:pubprop', 'PP');
      $query2->leftJoin('1:cvterm', 'T', '[PP].[type_id] = [T].[cvterm_id]');
      $query2->fields('PP', ['value']);
      $query2->fields('T', ['name']);
      $query2->condition('PP.pub_id', $pub_id, '=');
      $results2 = $query2->execute();
      foreach ($results2 as $prop) {
        $properties[$prop->name] = $prop->value;
      }

      // Build the citation with the citation manager service.
      if (!array_key_exists($pub_type, $this->citation_formats)) {
        $this->citation_formats[$pub_type] = $this->citation_manager->getDefaultCitationTemplate($pub_type);
      }
      $citation = $this->citation_manager->generateCitation($this->citation_formats[$pub_type], $properties);
    }

    return $citation;
  }

  /**
   * Publishes a single record in a single field to a drupal field table.
   *
   * @param string $field
   *   The field machine name, e.g. "pub_abstract".
   * @param int $condition_value
   *   The chado record id to use as the WHERE value.
   * @param string $update_value
   *   The new value for the update.
   * @param bool $drop
   *   If TRUE, then drop the record rather than update.
   *   In this case, the update value is ignored.
   *
   * @return string
   *   An empty string for success, otherwise an error message.
   */
  protected function microPublish(string $field, int $condition_value, string $update_value, bool $drop = FALSE): string {
    $table = 'tripal_entity__' . $field;
    $condition_column = $field . '_record_id';
    $update_column = $field . '_value';
    $status = '';
    $count = 0;
    try {
      if ($drop) {
        $query = $this->drupal_connection->drop($table);
      }
      else {
        $query = $this->drupal_connection->update($table);
        $query->fields([$update_column => $update_value]);
      }
      $query->condition($condition_column, $condition_value, '=');
      $count = $query->execute();
    }
    catch (\Exception $e) {
      $status .= "Exception for field \"$field\" record \"$condition_value\" " . $e->getMessage();
    }
    if ($count > 1) {
      $status .= " Error, more than one record ($count) was " . ($drop ? 'dropped' : 'updated') . " by microPublish for field \"$field\" record \"$condition_value\"";
    }
    return $status;
  }

}
