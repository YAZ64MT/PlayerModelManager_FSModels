# Player Model Manager Filesystem Addon

File system support for Player Model Manager.

Allows zobj models compiled for ModLoader64 to be read from this folder:

<recomp appdata folder>/mod_data/yazmt_z64_playermodelmanager/models/

Some of these models rely on assets stored inside the Ocarina of Time ROM, which may cause incorrect textures to be shown when they are loaded into Majora's Mask.

To resolve these issues, place a US or JP 1.0, 1.1, or 1.2 OoT ROM named oot.z64/oot.v64/oot.n64 into the yazmt_z64_playermodelmanager folder. This will allow this mod to extract the necessary assets from OoT.

If the extraction is successful, the corrupted textures will display correctly, and two new models, "Adult Link (OoT)" and "Young Link (OoT)", will appear in PlayerModelManager, and most categories of equipment will have an OoT version if available.

A large repository of models this addon supports can be found here:
https://github.com/YAZ64MT/ML64PlayerModels
