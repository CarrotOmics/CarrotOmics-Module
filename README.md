CarrotOmics-Module
==================

This is a custom Tripal module used on the CarrotOmics.org site.

It is a work-in-progress, and not ready for general use.

So far, this module consists of only Tripal Fields. These fields are:

sio__geographic_position
------------------------

This Tripal field can be used on a germplasm entity page to show geolocation,
altitude, and description. CarrotOmics uses a custom addition to the chado.nd_geolocation
table to include uncertainty values, used when the geolocation is only approximate.
![sio__geographic_position example image](/docs/sio__geographic_position_example.jpg?raw=true "Example")

iao__image
----------

This Tripal field can be used on a germplasm entity page to show all images of that
germplasm. This field uses a pager, so a large number of images can be handled.
![iao__image example image](/docs/iao__image_example.jpg?raw=true "Example")

edam__accession
---------------

This Tripal field can be used on an organism entity page to list all germplasm
accessions for that organism.
![edam__accession example image](/docs/edam__accession_example.jpg?raw=true "Example")

edam__phylogenetic_tree
-----------------------

This Tripal field can be used on an analysis entity page to list all phylogenetic
trees that are part of that analysis.
![edam__phylogenetic_tree example image](/docs/edam__phylogenetic_tree_example.jpg?raw=true "Example")


local__biomaterial_project
--------------------------

This Tripal field can be used on a biomaterial entity page to show projects and analyses
using this biomaterial.
![local__biomaterial_project example image](/docs/local__biomaterial_project_example.jpg?raw=true "Example")

local__analysis_project
-----------------------

This Tripal field can be used on an analysis entity page to show what project this
analysis belongs to.
![local__analysis_project example image](/docs/local__analysis_project_example.jpg?raw=true "Example")

local__project_analysis
-----------------------

This Tripal field can be used on a project entity page to show all analyses that
are part of the project.
![local__project_analysis example image](/docs/local__project_analysis_example.jpg?raw=true "Example")
