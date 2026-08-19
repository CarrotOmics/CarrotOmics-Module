<?php

namespace Drupal\carrotomics\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\Core\Link;
use Drupal\Core\Url;
use Drupal\tripal\Services\TripalEntityLookup;
use Drupal\tripal_chado\Database\ChadoConnection;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Generates the CarrotOmics Status Report page.
 */

class StatusReportController extends ControllerBase {

  /**
   * The Chado database connection.
   *
   * @var Drupal\tripal_chado\Database\ChadoConnection
   */
  protected ChadoConnection $chado_connection;

  /**
   * Tripal entity lookup service.
   *
   * @var Drupal\tripal\Services\TripalEntityLookup
   */
  protected TripalEntityLookup $entity_lookup_manager;

  /**
   * Controller constructor.
   */
  public function __construct(ChadoConnection $chado_connection, TripalEntityLookup $entity_lookup_manager) {
    $this->chado_connection = $chado_connection;
    $this->entity_lookup_manager = $entity_lookup_manager;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container): static {
    return new static(
      $container->get('tripal_chado.database'),
      $container->get('tripal.tripal_entity.lookup'),
    );
  }

  /**
   * Generates the CarrotOmics Status Report page.
   *
   * @return array
   *   The content render array.
   */
  public function content(): array {

    $elements = [];

    // Links used.
    $biosample_link = Link::fromTextAndUrl($this->t('biosample search page'), Url::fromUserInput('/data-search/biological-sample'))->toString();
    $collection_link = Link::fromTextAndUrl($this->t('collection search page'), Url::fromUserInput('/data-search/germplasm-collection'))->toString();
    $file_link = Link::fromTextAndUrl($this->t('file search page'), Url::fromUserInput('/data-search/tripal-file'))->toString();
    $genetic_map_link = Link::fromTextAndUrl($this->t('genetic map search page'), Url::fromUserInput('/data-search/genetic-map'))->toString();
    $genetic_marker_link = Link::fromTextAndUrl($this->t('genetic marker search page'), Url::fromUserInput('/data-search/genetic-marker'))->toString();
    $germplasm_link = Link::fromTextAndUrl($this->t('germplasm search page'), Url::fromUserInput('/data-search/all-germplasm'))->toString();
    $image_link = Link::fromTextAndUrl($this->t('image search page (not yet implemented)'), Url::fromUserInput('/data-search/images'))->toString();
    $mtl_link = Link::fromTextAndUrl($this->t('heritable phenotypic marker search page'), Url::fromUserInput('/data-search/heritable-phenotypic-marker'))->toString();
    $organism_link = Link::fromTextAndUrl($this->t('organism search page'), Url::fromUserInput('/data-search/organism'))->toString();
    $publication_link = Link::fromTextAndUrl($this->t('publication search page'), Url::fromUserInput('/data-search/publication'))->toString();
    $qtl_link = Link::fromTextAndUrl($this->t('QTL search page'), Url::fromUserInput('/data-search/qtl'))->toString();
    $tree_link = Link::fromTextAndUrl($this->t('tree search page'), Url::fromUserInput('/data-search/phylogenetic-tree'))->toString();

    /**
     * Organisms.
     */
    $n_organisms = $this->formatInt($this->chado_connection->select('1:organism')->countQuery()->execute()->fetchField());
    $n_families = $this->formatInt($this->chado_connection->query("SELECT COUNT(DISTINCT value) FROM {1:organismprop} WHERE type_id=(SELECT cvterm_id FROM {1:cvterm} WHERE name='superclass')")->fetchField());
    $n_genera = $this->formatInt($this->chado_connection->query("SELECT COUNT(DISTINCT genus) FROM {1:organism} WHERE NOT genus='N/A'")->fetchField());
    $elements['organisms'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' => $this->t('Organisms'),
      'body' => [
        '#markup' => $this->t('Currently @n_organisms organisms from @n_genera genera in @n_families families in the Apiales taxonomic order are present in CarrotOmics.
          They can be viewed at the @organism_link.<br>
          Both GRIN taxon and NCBI taxon identifier numbers are present as Cross References when available.<br>
          Organism pages list all available germplasm accessions and breeding stocks as well as biomaterial references, unless the number is greater than 100.
          For these organisms, view accessions using the @germplasm_link.',
          [
            '@n_organisms' => $n_organisms,
            '@n_genera' => $n_genera,
            '@n_families' => $n_families,
            '@organism_link' => $organism_link,
            '@germplasm_link' => $germplasm_link,
          ]
        ),
      ],
    ];

    /**
     * Germplasm and BioSamples.
     */
    $results = $this->chado_connection->query("SELECT SC.name, SCP.value, COUNT(*) AS count FROM {1:stockcollection} SC LEFT JOIN {1:stockcollectionprop} SCP ON SC.stockcollection_id=SCP.stockcollection_id LEFT JOIN {1:stockcollection_stock} SCS ON SC.stockcollection_id=SCS.stockcollection_id GROUP BY SC.name, SCP.value ORDER BY SCP.value");
    $rows = [];
    $acc_total = 0;
    foreach ($results as $result) {
      $name = $result->value;
      $abbr = $result->name;
      $acc_total = $acc_total + $result->count;
      $count = $this->formatInt($result->count);
      $rows[] = [$count, $name, $abbr];
    }
    $nbio = $this->chado_connection->select('1:biomaterial')->countQuery()->execute()->fetchField();
    $rows[] = [$this->formatInt($nbio), "\"BioSample\" accessions", ''];
    $acc_total = $acc_total + $nbio;
    $rows[] = [$this->formatInt($acc_total), 'Total', ''];

    $acc_total = $this->formatInt($acc_total);
    $sp_count = $this->formatInt($this->chado_connection->query("SELECT COUNT(DISTINCT organism_id) FROM {1:stock}")->fetchField());
    $fam_count = $this->formatInt($this->chado_connection->query("SELECT COUNT(DISTINCT P.value) FROM {1:stock} S LEFT JOIN {1:organismprop} P ON S.organism_id=P.organism_id WHERE P.type_id=(SELECT cvterm_id FROM {cvterm} WHERE name='superclass')")->fetchField());

    $elements['germplasm'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' => $this->t('Germplasm and BioSamples'),
      'body' => [
        '#type' => 'table',
        '#header' => ['Accessions', 'Collection Name', 'Collection Abbreviation'],
        '#rows' => $rows,
      ],
      'footer' => [
        '#markup' => $this->t('These @acc_total germplasm accessions and biosamples represent @sp_count different Apiales species or subspecies from @fam_count families.<br>
          View more information about the collections @collection_link.<br>
          Geolocation data or origin country is loaded when available.',
          [
            '@acc_total' => $acc_total,
            '@sp_count' => $sp_count,
            '@fam_count' => $fam_count,
            '@collection_link' => $collection_link,
          ]
        ),
      ],
    ];

    /**
     * NCBI Data.
     */
    $n_ncbi_biosamples = $this->formatInt($this->chado_connection->query("SELECT COUNT(*) FROM {1:biomaterialprop} B LEFT JOIN {1:cvterm} C ON B.type_id=C.cvterm_id WHERE C.name='full_ncbi_xml'")->fetchField());
    $n_ncbi_projects = $this->formatInt($this->chado_connection->query("SELECT COUNT(*) FROM {1:projectprop} P LEFT JOIN {1:cvterm} C ON P.type_id=C.cvterm_id WHERE C.name='full_ncbi_xml'")->fetchField());
    $n_ncbi_assemblies = $this->formatInt($this->chado_connection->query("SELECT COUNT(*) FROM {1:analysisprop} A LEFT JOIN {1:cvterm} C ON A.type_id=C.cvterm_id WHERE C.name='full_ncbi_xml'")->fetchField());
    $elements['ncbi'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' => $this->t('NCBI Data'),
      'body' => [
        '#markup' => $this->t('Currently @n_ncbi_biosamples NCBI biosamples, @n_ncbi_projects projects, and @n_ncbi_assemblies assemblies from the Apiales have been loaded into CarrotOmics.
          Biosamples can be searched using the @biosample_link, and biosample pages have links to Sequence Read Archive (SRA) accessions when those exist.',
          [
            '@n_ncbi_biosamples' => $n_ncbi_biosamples,
            '@n_ncbi_projects' => $n_ncbi_projects,
            '@n_ncbi_assemblies' => $n_ncbi_assemblies,
            '@biosample_link' => $biosample_link,
          ]
        ),
      ],
    ];

    /**
     * Phenotypic Data.
     */
    $n_quantitative = $this->formatInt($this->chado_connection->query("SELECT COUNT(DISTINCT CV.name) FROM {1:phenotype} P LEFT JOIN {1:cvterm} CV ON P.attr_id=CV.cvterm_id WHERE P.value ~ '^\-?\d+\.?\d*$'")->fetchField());
    $n_qualitative = $this->formatInt($this->chado_connection->query("SELECT COUNT(DISTINCT CV.name) FROM {1:phenotype} P LEFT JOIN {1:cvterm} CV ON P.attr_id=CV.cvterm_id WHERE P.value !~ '^\-?\d+\.?\d*$'")->fetchField());
    $n_mtl = $this->formatInt($this->chado_connection->query("SELECT COUNT(DISTINCT C.name) FROM {1:feature} F LEFT JOIN {1:feature_cvterm} FC ON F.feature_id=FC.feature_id LEFT JOIN {1:cvterm} C ON FC.cvterm_id=C.cvterm_id WHERE F.type_id = (SELECT cvterm_id FROM {1:cvterm} CVT WHERE CVT.name='heritable_phenotypic_marker' AND CVT.cv_id=(SELECT cv_id FROM {1:cv} CV WHERE CV.name='sequence'))")->fetchField());
    $n_qtl = $this->formatInt($this->chado_connection->query("SELECT COUNT(DISTINCT C.name) FROM {1:feature} F LEFT JOIN {1:feature_cvterm} FC ON F.feature_id=FC.feature_id LEFT JOIN {1:cvterm} C ON FC.cvterm_id=C.cvterm_id WHERE F.type_id = (SELECT cvterm_id FROM {1:cvterm} CVT WHERE CVT.name='QTL' AND CVT.cv_id=(SELECT cv_id FROM {1:cv} CV WHERE CV.name='sequence'))")->fetchField());
    $results = $this->chado_connection->query("SELECT C.name, COUNT(*) FROM {1:nd_experiment_phenotype} N LEFT JOIN {1:phenotype} P ON N.phenotype_id=P.phenotype_id LEFT JOIN {1:cvterm} C ON P.attr_id=C.cvterm_id GROUP BY C.name ORDER BY C.name");
    $rows = [];
    $pheno_total = 0;
    foreach ($results as $result) {
      $trait = $result->name;
      $count = $result->count;
      $pheno_total = $acc_total + $count;
      $rows[] = [$this->formatInt($count), $trait];
    }
    $pheno_total = $this->formatInt($pheno_total);
    $rows[] = [$pheno_total, 'Total'];

    $pheno_no_reps_count = $this->formatInt($this->chado_connection->query("SELECT COUNT(DISTINCT NE.nd_experiment_id) FROM {1:organism} O
      INNER JOIN {1:stock} S on S.organism_id = O.organism_id
      INNER JOIN {1:nd_experiment_stock} NES on NES.stock_id = S.stock_id
      INNER JOIN {1:nd_experiment_phenotype} NEP on NEP.nd_experiment_id = NES.nd_experiment_id
      INNER JOIN {1:nd_experiment} NE on NE.nd_experiment_id = NEP.nd_experiment_id
      INNER JOIN {1:nd_experiment_project} NEPR on NEPR.nd_experiment_id = NE.nd_experiment_id
      INNER JOIN {1:project} PR on PR.project_id = NEPR.project_id
      INNER JOIN {1:phenotype} P on P.phenotype_id = NEP.phenotype_id")->fetchField());
    $stock_count = $this->formatInt($this->chado_connection->query("SELECT COUNT(DISTINCT S.stock_id) FROM {1:nd_experiment_phenotype} P LEFT JOIN {1:nd_experiment_stock} E ON P.nd_experiment_id=E.nd_experiment_id LEFT JOIN {1:stock} S ON E.stock_id=S.stock_id")->fetchField());
    $org_count = $this->formatInt($this->chado_connection->query("SELECT COUNT(DISTINCT S.organism_id) FROM {1:nd_experiment_phenotype} P LEFT JOIN {1:nd_experiment_stock} E ON P.nd_experiment_id=E.nd_experiment_id LEFT JOIN {1:stock} S ON E.stock_id=S.stock_id")->fetchField());

    $elements['phenotypic_data'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' => $this->t('Phenotypic Data'),
      'header' => [
        '#markup' => $this->t('Observations of the following phenotypes are currently available in CarrotOmics.
          This list consists of @n_quantitative quantitative traits, @n_qualitative qualitative traits, @n_qtl QTL, and @n_mtl Mendelian trait loci',
          [
            '@n_quantitative' => $n_quantitative,
            '@n_qualitative' => $n_qualitative,
            '@n_mtl' => $n_mtl,
            '@n_qtl' => $n_qtl,
          ]
        ),
      ],
      'body' => [
        '#type' => 'table',
        '#header' => ['Observations', 'Trait'],
        '#rows' => $rows,
      ],
      'footer' => [
        '#markup' => $this->t('There are @pheno_no_reps_count phenotypic observations (@pheno_total counting replications) from @stock_count samples representing @org_count different species or subspecies.',
          [
            '@pheno_no_reps_count' => $pheno_no_reps_count,
            '@pheno_total' => $pheno_total,
            '@stock_count' => $stock_count,
            '@org_count' => $org_count,
          ]
        ),
      ],
    ];

    /**
     * Images.
     */
    $results = $this->chado_connection->query("SELECT P.value, COUNT(*) FROM {1:eimage} E LEFT JOIN {1:eimageprop} P ON E.eimage_id=P.eimage_id LEFT JOIN {1:cvterm} C ON P.type_id=C.cvterm_id WHERE C.name='dataset_name' GROUP BY P.value ORDER BY P.value");
    $rows = [];
    $image_total = 0;
    foreach ($results as $result) {
      $name = $result->value;
      $count = $result->count;
      $image_total = $image_total + $count;
      $rows[] = [$this->formatInt($count), $name];
    }
    $image_total = $this->formatInt($image_total);
    $rows[] = [$image_total, 'Total'];
    $elements['images'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' => $this->t('Images'),
      'header' => [
        '#markup' => $this->t('Images are viewable from individual germplasm accession pages, or searchable using the @image_link.<br>
          The Current status of images loaded in CarrotOmics is',
          [
            '@image_link' => $image_link,
          ]
        ),
      ],
      'body' => [
        '#type' => 'table',
        '#header' => ['Count', 'Source'],
        '#rows' => $rows,
      ],
    ];

    /**
     * Downloadable Files.
     */
    $results = $this->chado_connection->query("SELECT C.name, COUNT(*) AS count FROM {1:file} F LEFT JOIN {1:cvterm} C on F.type_id=C.cvterm_id GROUP BY C.name ORDER BY C.name");
    $rows = [];
    $file_total = 0;
    foreach ($results as $result) {
      $name = $result->name;
      $count = $result->count;
      $file_total = $file_total + $count;
      $rows[] = [$this->formatInt($count), $name];
    }
    $file_total = $this->formatInt($file_total);
    $rows[] = [$file_total, 'Total'];
    $elements['files'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' => $this->t('Downloadable Files'),
      'header' => [
        '#markup' => $this->t('Data files associated with content on CarrotOmics that are available for download can be searched on the @file_link.<br>
          The following types of downloadable files are available',
          [
            '@file_link' => $file_link,
          ]
        ),
      ],
      'body' => [
        '#type' => 'table',
        '#header' => ['Count', 'File Type'],
        '#rows' => $rows,
      ],
    ];

    /**
     * Publications.
     */
    $n_pubs = $this->formatInt($this->chado_connection->select('1:pub')->countQuery()->execute()->fetchField());
    $n_theses = $this->formatInt($this->chado_connection->query("SELECT COUNT(*) FROM {1:pub} WHERE type_id IN (SELECT cvterm_id FROM {1:cvterm} WHERE name IN ('Master''s Thesis', 'PhD Thesis', 'Thesis'))")->fetchField());
    $n_germ_rel = $this->formatInt($this->chado_connection->query("SELECT COUNT(*) FROM {1:pub} WHERE type_id IN (SELECT cvterm_id FROM {1:cvterm} WHERE name = 'Germplasm Release')")->fetchField());
    $elements['publications'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' => $this->t('Publications'),
      'body' => [
        '#markup' => $this->t('@n_pubs publications from both PubMed, the National Agricultural Library, and other sources matching selected search terms have been loaded into CarrotOmics, and are searchable through the @publication_link.<br>
          Specialty publication types such as theses (@n_theses) and variety release notices (@n_germ_rel) are also available.',
          [
            '@n_pubs' => $n_pubs,
            '@n_theses' => $n_theses,
            '@n_germ_rel' => $n_germ_rel,
            '@publication_link' => $publication_link,
          ]
        ),
      ],
    ];

    /**
     * Markers and Linkage Maps.
     */
    $n_maps = $this->formatInt($this->chado_connection->select('1:featuremap')->countQuery()->execute()->fetchField());
    $n_genetic_markers = $this->formatInt($this->chado_connection->query("SELECT COUNT(*) FROM {1:feature} WHERE type_id=(SELECT cvterm_id FROM {1:cvterm} WHERE name='genetic_marker')")->fetchField());
    $n_qtl = $this->formatInt($this->chado_connection->query("SELECT COUNT(*) FROM {1:feature} WHERE type_id=(SELECT cvterm_id FROM {1:cvterm} WHERE name='QTL')")->fetchField());
    $n_pheno = $this->formatInt($this->chado_connection->query("SELECT COUNT(*) FROM {1:feature} WHERE type_id=(SELECT cvterm_id FROM {1:cvterm} WHERE name='heritable_phenotypic_marker')")->fetchField());
    // Waiting for module migration: $n_corr = $this->formatInt($this->chado_connection->query("SELECT COUNT(*) FROM {0:tripal_map_correspondences}")->fetchField());
    $n_corr = 0;
    $elements['markers_maps'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' => $this->t('Markers and Linkage Maps'),
      'body' => [
        '#markup' => $this->t('@n_maps genetic linkage maps are present in CarrotOmics.
          @n_genetic_markers genetic markers, @n_qtl quantitative trait loci (QTL), and @n_pheno heritable mendelian trait loci (MTL) have been loaded and can be viewed with the Tripal Map Viewer.
          Maps are listed in the @genetic_map_link.<br>
          Search for genetic markers on the @genetic_marker_link. Search for QTL on the @qtl_link. Search for MTL on the @mtl_link.',
          // Waiting for module migration: @n_corr marker to genome position correspondences have been loaded.
          [
            '@n_maps' => $n_maps,
            '@n_genetic_markers' => $n_genetic_markers,
            '@n_qtl' => $n_qtl,
            '@n_pheno' => $n_pheno,
            // '@n_corr' => $n_corr,
            '@genetic_map_link' => $genetic_map_link,
            '@genetic_marker_link' => $genetic_marker_link,
            '@qtl_link' => $qtl_link,
            '@mtl_link' => $mtl_link,
          ]
        ),
      ],
    ];

    /**
     * Genomes.
     */
    $elements['genomes'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' =>  $this->t('Genomes'),
      'body' => [
        '#markup' => $this->t('Waiting on reimplementation of JBrowse and BLAST.'),
      ],
    ];

    /**
     * Resequencing, Variants.
     */
    $query = $this->chado_connection->select('1:file', 'F');
    $query->fields('F', ['file_id', 'name']);
    $query->leftJoin('1:cvterm', 'T', '"F".type_id = "T".cvterm_id');
    $query->leftJoin('1:cv', 'V', '"T".cv_id = "V".cv_id');
    $query->condition('T.name', 'VCF', '=');
    $query->condition('V.name', 'EDAM', '=');
    $results = $query->execute();
    $rows = [];
    foreach ($results as $result) {
      $name = $result->name;
      $entity_id = $this->entity_lookup_manager->getEntityId($result->file_id, NULL, NULL, 'file');
      if ($entity_id) {
        $name = Link::fromTextAndUrl($name, Url::fromUserInput('/bio_data/' . $entity_id))->toString();
      }
      $rows[] = [$name];
    }
    $elements['reseq'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' => $this->t('Resequencing, Variants'),
      'header' => [
        '#markup' => $this->t('The following sets of variants are available. Analysis descriptions or publications are avaliable from the linked files.'),
      ],
      'body' => [
        '#type' => 'table',
        '#header' => ['Variant File'],
        '#rows' => $rows,
      ],
    ];

    /**
     * Expression.
     */

    /**
     * Phylogenetic Trees.
     */
    $n_trees = $this->formatInt($this->chado_connection->select('1:phylotree')->countQuery()->execute()->fetchField());
    $n_pubs = $this->formatInt($this->chado_connection->query("SELECT COUNT(DISTINCT pub_id) FROM {1:phylotree_pub}")->fetchField());
    $elements['phylotrees'] = [
      '#type' => 'details',
      '#open' => TRUE,
      '#title' => $this->t('Phylogenetic Trees'),
      'body' => [
        '#markup' => $this->t('Currently @n_trees trees from @n_pubs publications have been loaded in CarrotOmics.<br>
          View them on the @tree_link.',
          [
            '@n_trees' => $n_trees,
            '@n_pubs' => $n_pubs,
            '@tree_link' => $tree_link,
          ]
        ),
      ],
    ];

    /**
     * Search Functions.
     */
    // All search functions in the Search menu are functional, including Tripal MegaSearch.
    /**
     * Data Downloads.
     */
    return $elements;
  }

  /**
   * Formats an integer with thousands separator.
   *
   * @param int
   *   The value to format, e.g. '1234567'.
   *
   * @return string
   *   The formatted integer, e.g. '1,234,567'.
   */
  protected function formatInt(int $value): string {
    return number_format($value, 0, '', ',');
  }

}
#      'heading' => [
#        '#type' => 'html_tag',
#        '#tag' => 'h3',
#        '#value' => $this->t('Phenotypic Data'),
#      ],
