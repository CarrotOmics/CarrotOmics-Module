CarrotOmics-Module
==================

This is a custom Tripal module used on the CarrotOmics.org site.

It is a work-in-progress, and not ready for general use.

Custom Tripal Fields in this module are:

sio__geographic_position
------------------------

This Tripal field can be used on a germplasm entity page to show geolocation,
altitude, and description. CarrotOmics uses a custom addition to the chado.nd_geolocation
table to include uncertainty values, used when the geolocation is only approximate.

![sio__geographic_position example image](/docs/sio__geographic_position_example.jpg?raw=true "Example image of sio__geographic_position tripal field")

iao__image
----------

This Tripal field can be used on a germplasm entity page to show all images of that
germplasm. This field uses a pager, so a large number of images can be handled.

![iao__image example image](/docs/iao__image_example.jpg?raw=true "Example image of iao__image tripal field")

edam__accession
---------------

This Tripal field can be used on an organism entity page to list all germplasm
accessions for that organism.

![edam__accession example image](/docs/edam__accession_example.jpg?raw=true "Example image of edam__accession tripal field")

edam__phylogenetic_tree
-----------------------

This Tripal field can be used on an analysis entity page to list all phylogenetic
trees that are part of that analysis.

![edam__phylogenetic_tree example image](/docs/edam__phylogenetic_tree_example.jpg?raw=true "Example image of edam__phylogenetic_tree tripal field")


local__biomaterial_project
--------------------------

This Tripal field can be used on a biomaterial entity page to show projects and analyses
using this biomaterial.

![local__biomaterial_project example image](/docs/local__biomaterial_project_example.jpg?raw=true "Example image of local__biomaterial_project tripal field")

local__analysis_project
-----------------------

This Tripal field can be used on an analysis entity page to show what project this
analysis belongs to.

![local__analysis_project example image](/docs/local__analysis_project_example.jpg?raw=true "Example image of local__analysis_project tripal field")

local__project_analysis
-----------------------

This Tripal field can be used on a project entity page to show all analyses that
are part of the project.

![local__project_analysis example image](/docs/local__project_analysis_example.jpg?raw=true "Example image of local__project_analysis tripal field")

local__stock_analysis
-----------------------

This Tripal field can be used on a stock entity page to show all analyses that
use this stock.

![local__stock_analysis example image](/docs/local__stock_analysis_example.jpg?raw=true "Example image of local__stock_analysis tripal field")

data__jbrowse_coordinates
-------------------------

This Tripal Field is a replacement for the `data__sequence_coordinates` field,
the only difference is that it will include a link to JBrowse.

![data__jbrowse_coordinates example image](/docs/data__jbrowse_coordinates_example.jpg?raw=true "Example image of data__jbrowse_coordinates tripal field")

main__stock_linker
------------------

This field is intended to be able to provide a link to any stock content type, such as
population, accession, and others, from other content types. Currently it supports a
link to population from a Tripal Map (featuremap) page. See `edam__genetic_map` for the
link in the reverse direction. This field uses the `chado.featuremap_stock` table to
obtain the link information.

![main__stock_linker example image](/docs/main__stock_linker_example.jpg?raw=true "Example image of main__stock_linker tripal field")

edam__genetic_map
-----------------

This field provides the inverse of the `main__stock_linker` field, it provides a link to
a Tripal Map (featuremap) page from a stock page. The current implementation supports a
link from a population page to a map using that population. This field uses the
`chado.featuremap_stock` table to obtain the link information.

![edam__genetic_map example image](/docs/edam__genetic_map_example.jpg?raw=true "Example image of edam__genetic_map tripal field")

