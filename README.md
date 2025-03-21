Recipes
========

Provides an administrative interface to manage recipe modules.

Supported actions:
- Apply recipe: Will import the configuration files from this recipe (equivilent to
    'Enable' on the modules page).
- Revert recipe: Will re-install the configuration files from this recipe, overwriting
    any changes.
- Inspect recipe: Will list all config files included in this recipe.
- Remove recipe: (Coming soon) Will remove all active config files originating from this recipe.
- Export recipe: (Coming soon) Will re-export the current state of the configuratuion
    files from this recipe, including any changes that have been made.


## Developer notes:

Include the following lines to your recipe module's .info file to have it appear
on the recipies listing page (admin/structure/recipes):
```
package = Recipes
```

To prevent the recipe from appearing on the modules lsting page (admin/modules),
you may add the following line. It will still be visible on the Recipes page.
```
hidden = true
```

Installation
------------

- Install this module using the official Backdrop CMS instructions at
  https://backdropcms.org/guide/modules

- Navigate to the Recipes page at Administer > Structure > Recipes
  (admin/structure/recipes) to view a list of all available recipes.


License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.


Current Maintainers
-------------------

- Jen Lampton (https://github.com/jenlampton)
- Seeking additional maintainers.


Credits
-------

- Craeated for Backdrop CMS by Jen Lampton (https://github.com/jenlampton).
