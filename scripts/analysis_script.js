const CUSTOMTOOL_SELECT = "rgb(226, 225, 240)";
document.addEventListener("DOMContentLoaded", function () {
    addPreviousProjectRow();
});
//fastp default setting
var defaultSettingFastp = {
    "Merge pair-end read": false,
    "Adapter trimming": true,
    "Poly-G trimming": true,
    "Minimum length": 15,
    "Maximum length": 0,
    "N-base limit": 5,
    "Phred score": 15,
};
//fastp compose setting
var composeSettingFastp = {
    "Merge pair-end read": false,
    "Adapter trimming": true,
    "Poly-G trimming": true,
    "Minimum length": 15,
    "Maximum length": 0,
    "N-base limit": 5,
    "Phred score": 15,
};

//cutadapt default setting
var defaultSettingCutadapt = {
    "Adapter type": "Regular adapter",
    "Adpter type index num": 0,
    "read5end checked": false,
    "read5end sequence": "",
    "read3end checked": false,
    "read3end sequence": "",
    "Phred score": 33,
    "Minimum length": 15,
    "Maximum length": 600,
    "N-base limit": 5,
    "Poly-A/poly-T trimming": false,
};

//cutadapt compose setting
var composeSettingCutadapt = {
    "Adapter type": "Regular adapter",
    "Adpter type index num": 0,
    "read5end checked": false,
    "read5end sequence": "",
    "read3end checked": false,
    "read3end sequence": "",
    "Phred score": 33,
    "Minimum length": 15,
    "Maximum length": 600,
    "N-base limit": 5,
    "Poly-A/poly-T trimming": false,
};

//kallisto default setting
var defaultSettingKallisto = {
    "Genome index name": "Create New Index",
    "Genome index num": 0,
    "Genome index upload file": {},
    "K-mers": 31,
    "Bootstrap samples": 0,
    "Seeds": 42,
    "Standard deviation of Fragment length": 0,
    "Fragment length": 0,
    "Single overhang": false,
    "Plain text": false,
};
//kallisto compose setting
var composeSettingKallisto = {
    "Genome index name": "Create New Index",
    "Genome index num": 0,
    "Genome index upload file": {},
    "K-mers": 31,
    "Bootstrap samples": 0,
    "Seeds": 42,
    "Standard deviation of Fragment length": 0,
    "Fragment length": 0,
    "Single overhang": false,
    "Plain text": false,
};

//subread default setting
var defaultSettingSubread = {
    "Genome index name": "Create New Index",
    "Genome index num": 0,
    "Genome index upload file": {},
    "Subreads threshold": 100,
    "Full index": false,
    "Sequencing data type": "DNA",
    "Sequecing data type index num": 0,
    "GTF index name": "Upload New GTF",
    "GTF index num": 0,
    "GTF index upload file": {},
    "Minimum fragment length": 50,
    "Maximum fragment length": 600,
    "Indels": 5,
    "Consensus subreads threshold": 3,
    "Maximum mismatches": 3,
    "Sort reads by coordinates": false,
};

//subread compose setting
var composeSettingSubread = {
    "Genome index name": "Create New Index",
    "Genome index num": 0,
    "Genome index upload file": {},
    "Subreads threshold": 100,
    "Full index": false,
    "Sequencing data type": "DNA",
    "Sequecing data type index num": 0,
    "GTF index name": "Upload New GTF",
    "GTF index num": 0,
    "GTF index upload file": {},
    "Minimum fragment length": 50,
    "Maximum fragment length": 600,
    "Indels": 5,
    "Consensus subreads threshold": 3,
    "Maximum mismatches": 3,
    "Sort reads by coordinates": false,
};

var projectStoredUUIDs = {}; //uuid hash-table for project table
var projectCollection = {};
var projectNameCollection = {};
var customToolStoredUUIDs = {}; //uuid hash-table for custom
var customSettingCollection = {};

//#region uuid generation

// Example function to check if a UUID is already used
function uuidIsUsed(uuid, uuidCollection) {
    // Check if the UUID is a key in the hash table
    return uuidCollection.hasOwnProperty(uuid);
}

function storeGeneratedUUID(uuid, uuidCollection) {
    // Store the UUID in the hash table with a value of 'true'
    uuidCollection[uuid] = true;
}

function generateProjectObject(
    uuid,
    projectName,
    pipelineName,
    pairedEnd,
    inputPath,
    outputPath,
    projectDate,
    finished,
    email,
) {
    var project = {
        uuid: uuid,
        project_name: projectName,
        pipeline_name: pipelineName,
        paired_end: pairedEnd,
        input_directory: inputPath,
        output_directory: outputPath,
        project_date: projectDate,
        finished: finished,
        email: email,
    };
    return project;
}

function UUIDTemplate() {
    // Get the current timestamp in milliseconds
    var timestamp = new Date().getTime();

    // Define the UUID template with placeholders for random characters
    var uuidTemplate = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
    // Replace 'x' and 'y' with random hexadecimal characters

    var uuid = uuidTemplate.replace(/[xy]/g, function (placeholder) {
        // Generate a random value between 0 and 15
        var randomValue = (timestamp + Math.random() * 16) % 16 | 0;

        // If the placeholder is 'x', use the random value as is; if 'y', perform specific bit manipulation
        var character;
        if (placeholder === "x") {
            character = randomValue;
        } else if (placeholder === "y") {
            // Perform specific bit manipulation for 'y'
            character = (randomValue & 0x3) | 0x8;
        }

        // Convert the result to a hexadecimal string
        return character.toString(16);
    });
    return uuid;
}

function generateUUID(uuidCollection) {
    var uuid = UUIDTemplate();
    while (uuidIsUsed(uuid, uuidCollection)) uuid = UUIDTemplate;
    // Store the generated UUID in the hash table
    storeGeneratedUUID(uuid, uuidCollection);
    return uuid;
}

//#endregion

//#region check clicked tab

function checkTabclicked(tabGroupID) {
    var tabGroup = document.getElementsByName(tabGroupID);
    var checkedTab = null;
    for (var i = 0; i < tabGroup.length; i++) {
        if (tabGroup[i].checked) {
            checkedTab = tabGroup[i].id;
        }
    }
    return checkedTab;
}

//#endregion
function copyObject(original) {
    var copied = {}; // Create a new empty object

    // Iterate over the keys in the original object
    for (var key in original) {
        // Check if the key is a property of the object itself (not inherited)
        if (original.hasOwnProperty(key)) {
            // Copy the key-value pair to the new object
            copied[key] = original[key];
        }
    }

    return copied; // Return the new object with copied key-value pairs
}

function returnNewDefaultSetting(tool) {
    var defaultSetting;
    switch (tool) {
        case "fastp": {
            return (defaultSetting = {
                "Merge pair-end read": false,
                "Adapter trimming": true,
                "Poly-G trimming": true,
                "Minimum length": 15,
                "Maximum length": 0,
                "N-base limit": 5,
                "Phred score": 15,
            });
            break;
        }
        case "cutadapt": {
            return (defaultSetting = {
                "Adapter type": "Regular adapter",
                "Adpter type index num": 0,
                "read5end checked": false,
                "read5end sequence": "",
                "read3end checked": false,
                "read3end sequence": "",
                "Phred score": 33,
                "Minimum length": 15,
                "Maximum length": 600,
                "N-base limit": 5,
                "Poly-A/poly-T trimming": false,
            });
            break;
        }
        case "kallisto": {
            return (defaultSetting = {
                "Genome index name": "Create New Index",
                "Genome index num": 0,
                "Genome index upload file": {},
                "K-mers": 31,
                "Bootstrap samples": 0,
                "Seeds": 42,
                "Fragment length": 0,
                "Standard deviation of Fragment length": 0,
                "Single overhang": false,
                "Plain text": false,
            });
        }
        case "subread": {
            return (defaultSetting = {
                "Genome index name": "Create New Index",
                "Genome index num": 0,
                "Genome index upload file": {},
                "Subreads threshold": 100,
                "Full index": false,
                "Sequencing data type": "DNA",
                "Sequecing data type index num": 0,
                "GTF index name": "Upload New GTF",
                "GTF index num": 0,
                "GTF index upload file": {},
                "Minimum fragment length": 50,
                "Maximum fragment length": 600,
                "Indels": 5,
                "Consensus subreads threshold": 3,
                "Maximum mismatches": 3,
                "Sort reads by coordinates": false,
            });
        }
    }
}

//#region sidebar setting and display

//set the display tab content
var pannelTable = document.querySelectorAll(".main-container>div");
pannelTable.forEach(function (container) {
    container.style.display = "none";
});
pannelTable[0].style.display = "flex";

// set the sidebar clicking effect
const links = document.querySelectorAll(".sidebar-list a");

links.forEach((link) => {
    link.addEventListener("click", function (event) {
        event.preventDefault();
        // Remove 'active' class from all links
        links.forEach((l) => l.classList.remove("active"));

        // Add 'active' class to the clicked link
        this.classList.add("active");
        var tabName = this.querySelector("h3").id.split("-")[0];
        console.log(tabName);
        var pannelTable = document.querySelectorAll(".main-container>div");
        pannelTable.forEach(function (container) {
            container.style.display = "none";
        });

        if (tabName == "logout") {
            //理論上logout-tab這邊可以去通知server做一些data的清理，但先直接轉址
            window.location.replace("/login.html");
        }
        document.getElementById(tabName).style.display = "flex";
    });
});
//#endregion

//#region custom-tab action

function displayTooltipContent(object) {
    var uuid = object.childNodes[0].nodeValue;
    var displayString = "";
    if (customSettingCollection.hasOwnProperty(uuid)) {
        var settings = customSettingCollection[uuid];
        console.log(settings);
        for (var key in settings) {
            if (settings.hasOwnProperty(key)) {
                var value = settings[key];
                var keyValueString;
                if (key.search("index num") < 0) {
                    if (key.search("file") >= 0) {
                        keyValueString = "file name: " + value.name + "<br>";
                    } else {
                        keyValueString = key + ": " + value + "<br>";
                    }
                    displayString += keyValueString;
                }
            }
        }
    }
    console.log(displayString);
    object.childNodes[1].innerHTML = displayString;
}

//tool-table first row listener
document.addEventListener("DOMContentLoaded", function () {
    var firstRow = document.querySelector("#tool-table tbody tr");
    var initialSettingsColumn = firstRow.querySelector(
        "td:nth-child(3)>.custom-setting-uuid",
    );

    var firstTool = firstRow
        .querySelector("select")
        .options[0].value.split("_")[0];
    var initialSetting;

    if (initialSettingsColumn.childNodes[0].nodeValue.search("checkuuid") >= 0) {
        if (
            customSettingCollection.hasOwnProperty(
                initialSettingsColumn.childNodes[0].nodeValue,
            )
        )
            initialSetting =
                customSettingCollection[initialSettingsColumn.childNodes[0].nodeValue];
        else {
            initialSetting = returnNewDefaultSetting(firstTool);
            var uuid = generateUUID(customToolStoredUUIDs);
            customSettingCollection[uuid] = initialSetting;
            initialSettingsColumn.childNodes[0].nodeValue = uuid;
            console.log(
                "initialSettingsColumn text:",
                initialSettingsColumn.childNodes[0].nodeValue,
            );
        }
    } else {
        initialSetting = returnNewDefaultSetting(firstTool);
        var uuid = generateUUID(customToolStoredUUIDs);
        customSettingCollection[uuid] = initialSetting;
        initialSettingsColumn.childNodes[0].nodeValue = uuid;
        console.log(
            "initialSettingsColumn text:",
            initialSettingsColumn.childNodes[0].nodeValue,
        );
    }

    firstRow.addEventListener("click", function (event) {
        // Check if the click was on the select element
        if (event.target.tagName !== "SELECT") {
            // Find the select element within the clicked row
            var selectElement = firstRow.querySelector("select");

            // Get the selected tool value
            var selectedTool = selectElement.value.split("_")[0];
            console.log("selectedTool:", selectedTool);

            //get the column of settings
            var settings;
            var settingsColumn = firstRow.querySelector(
                "td:nth-child(3)>.custom-setting-uuid",
            );
            if (settingsColumn.childNodes[0].nodeValue != "") {
                if (
                    customSettingCollection.hasOwnProperty(
                        settingsColumn.childNodes[0].nodeValue,
                    )
                )
                    settings =
                        customSettingCollection[settingsColumn.childNodes[0].nodeValue];
            } else {
                settings = returnNewDefaultSetting(selectedTool);
                var uuid = generateUUID(customToolStoredUUIDs);
                customSettingCollection[uuid] = settings;
                settingsColumn.childNodes[0].nodeValue = uuid;
            }
            deselectAllRows();
            firstRow.style.background = CUSTOMTOOL_SELECT;

            showToolSettings(selectedTool, "Custom-tab", settings);
        }
    });
});

//add row into custom-tab table with click addrow btn
function addRow(event, button) {
    var table = document.getElementById("tool-table");
    var currentRow = button.parentNode.parentNode;
    console.log(currentRow);
    currentRow = button.closest("tr");
    console.log(currentRow);
    var newRow = table.insertRow(currentRow.rowIndex + 1);
    var cells = currentRow.cells.length;

    for (var i = 0; i < cells; i++) {
        var newCell = newRow.insertCell(i);
        if (i === 0) {
            newCell.innerHTML = currentRow.cells[i].innerHTML;
            console.log(newCell.innerHTML);
        } else {
            console.log("cell index:", i);
            console.log(currentRow.cells[i].innerHTML);
            if (i == 2) {
                var firstTool = currentRow
                    .querySelector("select")
                    .options[0].value.split("_")[0];
                console.log("firstTool:", firstTool);

                initialSetting = returnNewDefaultSetting(firstTool);
                var uuid = generateUUID(customToolStoredUUIDs);
                customSettingCollection[uuid] = initialSetting;
                newCell.innerHTML = currentRow.cells[i].innerHTML;
                newCell.querySelector(".custom-setting-uuid").childNodes[0].nodeValue =
                    uuid;
            } else {
                newCell.innerHTML = currentRow.cells[i].innerHTML;
            }
        }
    }
    newRow.addEventListener("click", function (event) {
        var clickedCell = event.target.tagName;
        console.log("Tag name:", clickedCell);
        // Find the select element within the clicked row
        if (clickedCell != "SELECT") {
            var selectElement = newRow.querySelector("select");

            // Get the selected tool value
            var selectedTool = selectElement.value.split("_")[0];
            //get the column of settings
            var settings;
            var settingsColumn = newRow.querySelector(
                "td:nth-child(3)>.custom-setting-uuid",
            );

            if (settingsColumn.childNodes[0].nodeValue != "") {
                if (
                    customSettingCollection.hasOwnProperty(
                        settingsColumn.childNodes[0].nodeValue,
                    )
                )
                    settings =
                        customSettingCollection[settingsColumn.childNodes[0].nodeValue];
            } else {
                settings = returnNewDefaultSetting(selectedTool);
                var uuid = generateUUID(customToolStoredUUIDs);
                customSettingCollection[uuid] = settings;
                settingsColumn.childNodes[0].nodeValue = uuid;
            }

            //clear all background
            deselectAllRows();
            newRow.style.background = CUSTOMTOOL_SELECT;

            showToolSettings(selectedTool, "Custom-tab", settings);
        }
    });
    event.stopPropagation();
}

//delete row from custom-tab table with clicking deleteRow btn
function deleteRow(event, button) {
    var table = document.getElementById("tool-table");
    var currentRow = button.parentNode.parentNode.parentNode;
    var uuid = currentRow.querySelector(".custom-setting-uuid").childNodes[0]
        .nodeValue;
    console.log("delete row uuid: ", uuid);

    if (table.rows.length > 2) {
        if (customSettingCollection.hasOwnProperty(uuid)) {
            delete customSettingCollection[uuid];
            console.log(`delete uuid ${uuid} compeleted`);
        }
        table.deleteRow(currentRow.rowIndex);
    }

    event.stopPropagation();
}

//#endregion custom-tab action

//#region pipeline selective tab action

function changeTab(contentId, tableSelect) {
    var contents = document.querySelectorAll(`.${tableSelect} > div`);
    contents.forEach(function (content) {
        content.style.display = "none";
    });

    var selectedContent = document.getElementById(contentId);
    if (selectedContent) {
        selectedContent.style.display = "flex";
    }
    var toolSettingsContainers = document.querySelectorAll(".setting-box>div");
    if (contentId == "RNA-Seq-content") {
        toolSettingsContainers.forEach(function (container) {
            container.style.display = "none";
        });
    } else if (contentId == "Custom-content") {
        toolSettingsContainers.forEach(function (container) {
            container.style.display = "none";
        });
        var rows = document.querySelectorAll("#tool-table tbody tr");
        for (var i = 0; i < rows.length; i++) {
            rows[i].style.backgroundColor = "";
        }
    }
}

function showTabContent(contentId, tableSelect) {
    switch (tableSelect) {
        case "projects-tab-custom": {
            changeTab(contentId, tableSelect);
            break;
        }
        case "tab-content": {
            changeTab(contentId, tableSelect);
            break;
        }
        default:
            return;
    }
}

//#endregion

function returnRNASeqSetting(tool, getDefaultSetting) {
    if (getDefaultSetting == true) {
        switch (tool) {
            case "fastp": {
                return defaultSettingFastp;
                break;
            }
            case "cutadapt": {
                return defaultSettingCutadapt;
                break;
            }
            case "kallisto": {
                return defaultSettingKallisto;
                break;
            }
            case "subread": {
                return defaultSettingSubread;
                break;
            }
        }
    } else {
        switch (tool) {
            case "fastp": {
                return composeSettingFastp;
                break;
            }
            case "cutadapt": {
                return composeSettingCutadapt;
                break;
            }
            case "kallisto": {
                return composeSettingKallisto;
                break;
            }
            case "subread": {
                return composeSettingSubread;
                break;
            }
        }
    }
}

function inputTypeEditAction(
    object,
    inputType,
    settings,
    key,
    tool,
    defaultSettings,
) {
    switch (inputType) {
        case "checkbox": {
            settings[key] = object.checked;
            break;
        }
        case "text": {
            if (object.value.length > 0) settings[key] = object.value;
            else object.value = defaultSettings[key];

            break;
        }
        case "number": {
            if (object.value.length > 0)
                settings[key] = parseInt(object.value, 10);
            else
                object.value = defaultSettings[key];
            break;
        }
        case "file": {
            settings[key] = object.files[0];
            if (tool == "subread") {
                if (key.search("Genome") >= 0) {
                    document.getElementById("genome-index-select-subread").selectedIndex =
                        0;
                    settings["Genome index num"] = 0;
                    settings["Genome index name"] = "Create New Index";
                } else if (key.search("GTF") >= 0) {
                    document.getElementById("genome-index-select-subread").selectedIndex =
                        0;
                    settings["GTF index num"] = 0;
                    settings["GTF index name"] = "Upload New GTF";
                }
            } else if (tool == "kallisto") {
                if (key.search("Genome") >= 0) {
                    document.getElementById(
                        "genome-index-select-kallisto",
                    ).selectedIndex = 0;
                    settings["Genome index num"] = 0;
                    settings["Genome index name"] = "Create New Index";
                }
            }
            break;
        }
        case "select": {
            if (object.id == "adapter-select") {
                var read5EndChecked =
                    document.getElementById("read-5-end-check").checked;
                var read3EndChecked =
                    document.getElementById("read-3-end-check").checked;
                if (read5EndChecked == true && read3EndChecked == true)
                    settings[key] = "Linked adapter";
                else {
                    settings[key] = object.options[object.selectedIndex].value;
                    settings["Adapter type num"] = object.selectedIndex;
                }
            } else if (object.id == "subread-squence-type-select") {
                settings[key] = object.options[object.selectedIndex].value;
                settings["Sequecing data type index num"] = object.selectedIndex;
            } else if (object.id.search("genome") >= 0) {
                // if (object.options[object.selectedIndex].value.search('New') >= 0) {

                //   var element = document.getElementById('file-name-' + tool + '-index');
                //   console.log(element.id);
                //   element.innerText = "";
                //   element.style.background = 'white';
                //   settings["Genome index upload file"] = {};
                // }
                //clear file name content
                var element = document.getElementById("file-name-" + tool + "-genome");
                console.log(element.id);
                element.innerText = "";
                element.style.background = "";
                settings["Genome index upload file"] = {};

                settings[key] = object.options[object.selectedIndex].value;
                settings["Genome index num"] = object.selectedIndex;
            } else if (object.id.search("gtf") >= 0) {
                console.log(object.id);
                console.log(object.id.search("gtf"));
                // if (object.options[object.selectedIndex].value.search('New') >= 0) {

                //   var element = document.getElementById('file-name-' + tool + '-gtf');
                //   console.log(element.id);
                //   element.innerText = "";
                //   element.style.background = 'white';
                //   settings["GTF index upload file"] = {};

                // }
                var element = document.getElementById("file-name-" + tool + "-gtf");
                console.log(element.id);
                element.innerText = "";
                element.style.background = "";
                settings["GTF index upload file"] = {};

                settings[key] = object.options[object.selectedIndex].value;
                settings["GTF index num"] = object.selectedIndex;
            } else {
                console.log(object.id);
            }

            break;
        }
    }
}

function editSetting(object, key, tool) {
    var inputType;
    var objectType = object.tagName.toLowerCase();

    if (objectType == "input") inputType = object.type;
    else if (objectType == "select") inputType = "select";

    var checkedTab = checkTabclicked("tabGroup1");
    switch (checkedTab) {
        case "RNA-Seq-tab": {
            var settings = returnRNASeqSetting(tool, false);
            var defaultSettings = returnRNASeqSetting(tool, true);
            inputTypeEditAction(
                object,
                inputType,
                settings,
                key,
                tool,
                defaultSettings,
            );
            break;
        }
        case "Custom-tab": {
            var rows = document.querySelectorAll("#tool-table tbody tr");
            var settings = null;
            var defaultSettings = returnRNASeqSetting(tool, true);

            for (var i = 0; i < rows.length; i++) {
                //var computedStyle = window.getComputedStyle(rows[i]);
                //var backgroundColor = computedStyle.backgroundColor;
                var backgroundColor = rows[i].style.backgroundColor;
                console.log("Computed Background Color:", backgroundColor);
                console.log("CUSTOMTOOL_SELECT:", CUSTOMTOOL_SELECT);
                if (backgroundColor == CUSTOMTOOL_SELECT) {
                    selectColumn = rows[i].querySelector(
                        "td:nth-child(3)>.custom-setting-uuid",
                    );
                    var uuid = selectColumn.childNodes[0].nodeValue;
                    if (customSettingCollection.hasOwnProperty(uuid))
                        settings = customSettingCollection[uuid];
                    break;
                }
            }

            if (settings != null) {
                inputTypeEditAction(
                    object,
                    inputType,
                    settings,
                    key,
                    tool,
                    defaultSettings,
                );
            }
            break;
        }
    }
}

function displayFileName(input, value) {
    if (input.files.length > 0 && input.files) {
        var fileName;
        if (input.files.length > 1) {
            fileName = " (multi-file) " + input.files[0].name + "...";
        } else fileName = input.files[0].name;
        var element = document.getElementById("file-name-" + value);
        console.log(element.id);
        element.innerText = fileName;
        element.style.background = "lightgrey";

        var inputID = input.id;
        if (inputID.search("kallisto") >= 0 && inputID.search("genome") >= 0) {
            editSetting(input, "Genome index upload file", "kallisto");
        } else if (inputID.search("subread") >= 0) {
            if (inputID.search("genome") >= 0)
                editSetting(input, "Genome index upload file", "subread");
            else if (inputID.search("gtf") >= 0)
                editSetting(input, "GTF index upload file", "subread");
        }
    }
}

function toggleBoxCompose(settings, toggleBoxSetting) {
    for (var i = 0; i < toggleBoxSetting.length; i++) {
        //var toggleTitle = toggleBoxSetting[i].querySelector(".toggle-box-title").textContent;
        var toggleTitle = toggleBoxSetting[i].querySelector(".toggle-box-title");
        //var toggleChecked = toggleBoxSetting[i].querySelector(".toggle>input").checked;
        var toggleChecked = toggleBoxSetting[i].querySelector(".toggle>input");
        if (settings.hasOwnProperty(toggleTitle.textContent)) {
            toggleChecked.checked = settings[toggleTitle.textContent];
            console.log(
                `toggleTitle: ${toggleTitle.textContent}, toggleChecked: ${toggleChecked.checked}`,
            );
        }
    }
}

function uploadFileCompose(settings, tool) {
    var toolSettingID = "tool-setting-" + tool;
    var selector = document.getElementById(toolSettingID);
    var indexArray = Array.from(selector.querySelectorAll(".index-upload"));
    for (var i = 0; i < indexArray.length; i++) {
        var selectObject = indexArray[i].querySelector(".index-upload-btn>select");
        var indexType = selectObject.id.split("-")[0];
        switch (indexType) {
            case "genome": {
                selectObject.selectedIndex = settings["Genome index num"];
                var file = settings["Genome index upload file"];

                if (Object.keys(file).length != 0) {
                    var fileNameObjectID = "file-name-" + tool + "-genome";
                    var fileNameObject = document.getElementById(fileNameObjectID);
                    fileNameObject.textContent = file.name;
                    fileNameObject.style.background = lightgrey;
                }

                break;
            }
            case "GTF": {
                selectObject.selectedIndex = settings["GTF index num"];
                var file = settings["GTF index upload file"];

                if (file != null) {
                    var fileNameObjectID = "file-name-" + tool + "-gtf";
                    var fileNameObject = document.getElementById(fileNameObjectID);
                    fileNameObject.textContent = file.name;
                    fileNameObject.style.background = lightgrey;
                }
            }
        }
    }
}

function optionCompose(settings, optionSetting) {
    for (var i = 0; i < optionSetting.length; i++) {
        var optionTitle = optionSetting[i].querySelector(".option-title");
        console.log("optionTitle_original: ", optionTitle.textContent);
        var optionValue;
        if (settings.hasOwnProperty(optionTitle.textContent)) {
            if (optionSetting[i].querySelector(".option-select")) {
                var selector = optionSetting[i].querySelector(".option-select");
                for (var j = 0; j < selector.options.length; j++) {
                    if (selector.options[i].value == settings[optionTitle.textContent]) {
                        selector.selectedIndex = i;
                        console.log(
                            `optionTitle: ${optionTitle.textContent}, optionValue: ${selector.options[i].value}`,
                        );
                    }
                }
            } else {
                optionValue = optionSetting[i].querySelector("input");
                optionValue.value = settings[optionTitle.textContent];
                console.log(
                    `optionTitle: ${optionTitle.textContent}, optionValue: ${optionValue.value}`,
                );
            }
        }
    }
}

function getUploadFileSetting(tool) {
    var selector = document.querySelector(tool);
    var indexArray = Array.from(selector.querySelectorAll(".index-upload"));

    for (var i = 0; i < indexArray.length; i++) {
        var itemSelect = indexArray[i].querySelector(".index-upload-btn>select");
        var itemSelectID = itemSelect.selectedIndex;
        var itemSelectName = itemSelect.options[itemSelectID].value;
        if (itemSelectName.search("New") >= 0) {
            var uploadFileName =
                indexArray[i].querySelector(".index-file-name").textContent;
            if (uploadFileName != "") {
                console.log(
                    `uploadSelect:${itemSelectName}, uploadName:${uploadFileName}`,
                );
            } else {
                console.log(`uploadSelect:${itemSelectName}, uploadName:No_content`);
            }
        } else console.log(`uploadSelect:${itemSelectName}`);
    }
}

function getToolSettingTemp(tool, tab, settings) {
    var toolSettingID = "tool-setting-" + tool;
    var baseClass = document.getElementById(toolSettingID);
    var toggleBoxSetting = Array.from(baseClass.querySelectorAll(".toggle-box"));
    var optionSetting = Array.from(baseClass.querySelectorAll(".options"));
    switch (tab) {
        case "RNA-Seq-tab": {
            toggleBoxCompose(settings, toggleBoxSetting);
            optionCompose(settings, optionSetting);
            uploadFileCompose(settings, tool);
            if (tool == "cutadapt") {
                var adapterSelect = document.getElementById("adapter-select");
                var read5EndChecked = document.getElementById("read-5-end-check");
                var read3EndChecked = document.getElementById("read-3-end-check");
                var read5EndSequence = document.getElementById("read5end-sequence");
                var read3EndSequence = document.getElementById("read3end-sequence");

                read5EndChecked.checked = settings["read5end checked"];
                read3EndChecked.checked = settings["read3end checked"];
                read5EndSequence.value = settings["read5end sequence"];
                read3EndSequence.value = settings["read3end sequence"];
                if (read5EndChecked != true && read5EndChecked != true)
                    adapterSelect.selectedIndex = settings["Adapter type index num"];
            }
            break;
        }

        case "Custom-tab": {
            toggleBoxCompose(settings, toggleBoxSetting);
            optionCompose(settings, optionSetting);
            uploadFileCompose(settings, tool);
            if (tool == "cutadapt") {
                var adapterSelect = document.getElementById("adapter-select");
                var read5EndChecked = document.getElementById("read-5-end-check");
                var read3EndChecked = document.getElementById("read-3-end-check");
                var read5EndSequence = document.getElementById("read5end-sequence");
                var read3EndSequence = document.getElementById("read3end-sequence");

                read5EndChecked.checked = settings["read5end checked"];
                read3EndChecked.checked = settings["read3end checked"];
                read5EndSequence.value = settings["read5end sequence"];
                read3EndSequence.value = settings["read3end sequence"];
                if (read5EndChecked != true && read5EndChecked != true)
                    adapterSelect.selectedIndex = settings["Adapter type index num"];
            }
            break;
        }
    }
}

function deselectAllRows() {
    var rows = document.querySelectorAll("#tool-table tbody tr");
    //clear all background
    for (var i = 0; i < rows.length; i++) {
        rows[i].style.backgroundColor = "";
    }
}

function customToolSelectChange(event, selectObject) {
    var selectedRow = event.target.closest("tr");
    var settingColumn = selectedRow.querySelector(
        "td:nth-child(3)>.custom-setting-uuid",
    );
    //Create new uuid for new default setting tool key-value pair;
    var uuid = generateUUID(customToolStoredUUIDs);
    //get the selected tool name
    var selectedTool = selectObject.value.split("_")[0];
    //if user change the selectindex,then return a new default setting of selected tool
    var defaultSetting = returnNewDefaultSetting(selectedTool);
    //add new key-value pair
    customSettingCollection[uuid] = defaultSetting;
    //check the customSettingCollection, if previous uuid existed,delete the key-value pair
    if (
        customSettingCollection.hasOwnProperty(
            settingColumn.childNodes[0].nodeValue,
        )
    )
        delete customSettingCollection[settingColumn.childNodes[0].nodeValue];
    //set the column to reserve the uuid;
    settingColumn.childNodes[0].nodeValue = uuid;
    deselectAllRows();
    selectedRow.style.backgroundColor = CUSTOMTOOL_SELECT;
    showToolSettings(selectedTool, "Custom-tab", defaultSetting);
}

function showToolSettings(toolId, tab, settings) {
    // Hide all tool settings
    var toolSettingsContainers = document.querySelectorAll(".setting-box>div");
    toolSettingsContainers.forEach(function (container) {
        container.style.display = "none";
    });

    // Show the selected tool setting
    var selectedToolSetting = document.getElementById("tool-setting-" + toolId);
    if (selectedToolSetting) {
        getToolSettingTemp(toolId, tab, settings);
        selectedToolSetting.style.display = "block";
    }
}

//#region popupWindow setting

function popupWindow(window, bool) {
    var popwindow = document.getElementById(window);
    if (bool == true) popwindow.style.display = "flex";
    else popwindow.style.display = "none";
    checkStateStyle("", "reset", "project");
}

//#endregion

async function sendProjectExecuteRequest(project) {
    var header = {
        "Content-type": "application/json",
    };
    if (project["paired_end"] == true) {
        project["paired_end"] = "true";
    } else {
        project["paired_end"] = "false";
    }
    var body = JSON.stringify(project);
    var fetchResult = await fetchAPI("/project_execute", "POST", header, body);
    if (fetchResult["error"] != "") {
        console.log("error:", error);
        return;
    } else {
        console.log(fetchResult["data"]["message"]);
    }
    if ((fetchResult["data"]["message"].search('Error')) >= 0) {
        console.log(fetchResult["data"]["message"]);
        return;
    }

    var uuid = project["uuid"];

    if (fetchResult["data"]["message"].trim().search("finished") >= 0) {
        var project = projectCollection[uuid];
        project["finished"] = "true";
        // Find the table element
        var table = document.getElementById("new-table"); // Replace 'yourTableId' with the ID of your table

        // Loop through each row in the table
        for (var i = 0; i < table.rows.length; i++) {
            var row = table.rows[i];
            console.log(row);
            console.log(row.cells[0].textContent);
            console.log(row.cells[3].textContent);

            // Check if the first cell in the current row contains the value "1234"
            if (row.cells[0].textContent.trim() === uuid) {
                // Update the style class and text content of the fourth cell in this row
                var cell4 = row.cells[3];
                var runningdiv = cell4.querySelector('.status');
                runningdiv.className = "finished-status"; // Replace 'yourNewClass' with the new class name
                runningdiv.textContent = "finished"; // Replace 'New Content' with the new text content
                break; // Exit the loop since we found the row
            }
        }
    } else {
        console.log("doesn't reach if statement");
    }
}

//#region addrow in project table
async function addProjectRow() {
    checkStateStyle("", "reset", "project");
    var email = document.getElementById("user-account").textContent;
    var cancelBtn = document.getElementById("popCancelBtn");
    var projectName = document.getElementById("project-name-input-value").value;
    var date = new Date();
    var dateString =
        date.getFullYear() + "-" + (date.getMonth() + 1) + "-" + date.getDate();
    var pipeline = document.getElementById("project-pipeline-input-value").value;
    var uploadfiles = document.getElementById("raw-data-upload");
    var pairEndCheck = document.getElementById("paired-end-checkbox").checked;

    if (projectName == "") {
        checkStateStyle("Please fill the project Name", "error", "project");
        return;
    }
    if (projectNameCollection.hasOwnProperty(projectName)) {
        checkStateStyle("Duplicated project Name", "error", "project");
        return;
    }
    if (uploadfiles.files.length == 0) {
        checkStateStyle("Please select file", "error", "project");
        return;
    }
    cancelBtn.disabled = true;

    for (var i = 0; i < uploadfiles.files.length; i++) {
        var file = uploadfiles.files[i];
        var fileContent = await readFileAsync(file);
        if (fileContent instanceof ArrayBuffer)
            console.log(`${file.name} read to ArrayBuffer succeed`);
        else {
            checkStateStyle(`${file.name} read failed`, "error", "project");
            cancelBtn.disabled = false;
            return;
        }
        checkStateStyle(
            `uploading files(${i}/${uploadfiles.files.length}) ${file.name} ...`,
            "loading",
            "project",
        );
        var headers = {
            "Content-Type": "application/octet-stream",
            fileName: file.name,
            email: email,
            projectName: projectName,
        };
        console.log(headers);
        var body = fileContent;
        var fileUploadResult = await fetchAPI(
            "/project_fileUpload",
            "POST",
            headers,
            body,
        );

        if (fileUploadResult["error"] != "") {
            checkStateStyle(fileUploadResult["error"], "error", "project");
            cancelBtn.disabled = false;
            return;
        }
        var uploadMSG = fileUploadResult["data"]["message"];
        if (uploadMSG.search("Upload succeed") < 0) {
            checkStateStyle(uploadMSG, "error", "project");
            cancelBtn.disabled = false;
            return;
        }
    }

    checkStateStyle("store project...", "loading", "project");

    var table = document.getElementById("new-table");

    var uuid = generateUUID(projectStoredUUIDs);
    var project = generateProjectObject(
        uuid,
        projectName,
        pipeline,
        pairEndCheck,
        `/data/ngsase_data/project_rawdata/${email}/${projectName}`,
        `/data/ngsase_data/project_result/${email}/${projectName}`,
        "",
        "false",
        email
    );
    console.log(project);

    var header = {
        "Content-type": "application/json",
    };
    var body = JSON.stringify({
        uuid: uuid,
        project_name: projectName,
        pipeline_name: pipeline,
        paired_end: pairEndCheck == true ? "true" : "false",
        input_directory: `/data/ngsase_data/project_rawdata/${email}/${projectName}`,
        output_directory: `/data/ngsase_data/project_result/${email}/${projectName}`,
        project_date: "",
        finished: "false",
        email: email,
    });
    var fetchResult = await fetchAPI("/project_insert", "POST", header, body);
    if (fetchResult["error"] != "") {
        checkStateStyle(fetchResult["error"], "error", "project");
        cancelBtn.disabled = false;
        return;
    }
    if (fetchResult["data"].hasOwnProperty("message")) {
        var response = fetchResult["data"]["message"];
        if (response.search("succeed") < 0) {
            checkStateStyle(response, "error", "project");
            cancelBtn.disabled = false;
            return;
        } else
            checkStateStyle("project construction succeed", "finished", "project");
    }

    projectCollection[uuid] = project;
    projectNameCollection[projectName] = true;

    // Create a new row
    var newRow = table.insertRow(table.rows.length);
    var cell1 = newRow.insertCell(0);
    var cell2 = newRow.insertCell(1);
    var cell3 = newRow.insertCell(2);
    var cell4 = newRow.insertCell(3);
    var cell5 = newRow.insertCell(4);
    cell1.innerHTML = `<div>${uuid}</div>`;
    cell2.innerHTML = projectName;
    cell3.innerHTML = dateString;
    cell4.innerHTML = `<div class=status>Running</div>`;
    cell5.innerHTML = `<div><label class="download-btn" for="${uuid}" onclick="downloadFile('${uuid}')"><span class="material-symbols-outlined">download</span></label><button id="${uuid}" style="display:none" ></button></div>`;
    cancelBtn.disabled = false;
    sendProjectExecuteRequest(project);
    popupWindow("add-project-window", false);
}

async function downloadFile(uuid) {
    var project;
    var filePath = "";
    var finished = "";

    if (projectCollection.hasOwnProperty(uuid)) project = projectCollection[uuid];
    else {
        console.log(`the project ${uuid} doesn't exsited`);
        return;
    }

    if (project.hasOwnProperty("finished")) finished = project["finished"];

    if (finished != "") {
        if (finished == "false") {
            console.log(`the project ${uuid} haven't been finished yet`);
            return;
        }
    }

    if (project.hasOwnProperty("output_directory")) {
        filePath = project["output_directory"];
    } else {
        console.log(`There is no output_directory in your project ${uuid}`);
        return;
    }

    var headers = {
        "Content-type": "application/json",
    };
    var body = JSON.stringify({
        email: project["email"],
        project_name: project["project_name"],
        uuid: project["uuid"],
    });
    /*   fetch('/project_fileDownload', {
           method: 'POST',
           headers:headers,
           body:body
           // 其他設定或是需要傳遞的資料
       })
           .then((response) => response.blob())
           .then((blob) => {
               var url = window.URL.createObjectURL(blob); // create url from blob
               var fileLink = document.createElement('a'); // create link for file
               fileLink.href = url;
               fileLink.download = 'test.zip'; // download filename
               document.body.appendChild(fileLink); // append file link to download
               fileLink.click();
               fileLink.remove(); // remove file link after click
           })
           .catch((error) => {
               // Handle error here.
           });*/

    var response = await fetchAPI("/project_fileDownload", "POST", headers, body);

    console.log(response);
    if (response["error"] == "") {
        if (response["dataType"] === 'arraybuffer') {
            /* var fileName = response["fileName"];

             // Create a temporary anchor element
             var link = document.createElement("a");
             link.href = window.URL.createObjectURL(response["data"]);

             // Set the file name
             link.setAttribute("download", fileName);

             // Simulate a click to trigger the download
             link.click();
             // Clean up
             window.URL.revokeObjectURL(link.href);
             //因為沒有append到body，只是臨時的element，不用removeChild
             //link.parentNode.removeChild(link);*/

            var fileName = response["fileName"];
            var data = new Blob([response["data"]], {type: 'application/octet-stream'});

            // Create a temporary anchor element
            var link = document.createElement("a");
            link.href = window.URL.createObjectURL(data);

            // Set the file name
            link.setAttribute("download", fileName);

            // Simulate a click to trigger the download
            link.click();
            // Clean up
            window.URL.revokeObjectURL(link.href);
        } else if (response["dataType"] == "json") {
            consolo.log(response.data);
        }
    }
}

async function addPreviousProjectRow() {
    var table = document.getElementById("previous-table");
    var accountName = document.getElementById("user-account").textContent;
    console.log(accountName);
    var headers = {
        "Content-type": "application/json",
    };
    var body = {
        email: accountName,
    };
    body = JSON.stringify(body);
    var projectPreviousResults = await fetchAPI(
        "/project_previous",
        "POST",
        headers,
        body,
    );
    if (projectPreviousResults["error"] != "") {
        console.log("Error: ", projectPreviousResults["error"]);
        return;
    }

    for (var i = 0; i < projectPreviousResults["data"].length; i++) {
        var uuid;
        var project_name;
        var pipeline_name;
        var paried_end;
        var input_directory;
        var output_directory;
        var project_date;
        var finished;
        var email;

        var projectPrevious = projectPreviousResults.data[i];
        if (projectPrevious.hasOwnProperty("uuid")) uuid = projectPrevious["uuid"];
        if (projectPrevious.hasOwnProperty("project_name"))
            project_name = projectPrevious["project_name"];
        if (projectPrevious.hasOwnProperty("pipeline_name"))
            pipeline_name = projectPrevious["pipeline_name"];
        if (projectPrevious.hasOwnProperty("paired_end"))
            paried_end = projectPrevious["paired_end"];
        if (projectPrevious.hasOwnProperty("input_directory"))
            input_directory = projectPrevious["input_directory"];
        if (projectPrevious.hasOwnProperty("output_directory"))
            output_directory = projectPrevious["output_directory"];
        if (projectPrevious.hasOwnProperty("project_date"))
            project_date = projectPrevious["project_date"];
        if (projectPrevious.hasOwnProperty("finished"))
            finished = projectPrevious["finished"];
        if (projectPrevious.hasOwnProperty("email"))
            email = projectPrevious["email"];

        var project = generateProjectObject(
            uuid,
            project_name,
            pipeline_name,
            paried_end,
            input_directory,
            output_directory,
            project_date,
            finished,
            email,
        );
        projectCollection[uuid] = project;
        projectNameCollection[project["project_name"]] = true;
        storeGeneratedUUID(uuid, projectStoredUUIDs);
        var newRow = table.insertRow(table.rows.length);
        var cell1 = newRow.insertCell(0);
        var cell2 = newRow.insertCell(1);
        var cell3 = newRow.insertCell(2);
        var cell4 = newRow.insertCell(3);
        var cell5 = newRow.insertCell(4);
        cell1.innerHTML = `<div>${uuid}</div>`;
        cell2.innerHTML = project_name;
        cell3.innerHTML = project_date;
        if (finished == "true")
            cell4.innerHTML = `<div class=finished-status>Finished</div>`;
        else cell4.innerHTML = `<div class=status>Running</div>`;
        cell5.innerHTML = `<div><label class="download-btn" for="${uuid}" onclick="downloadFile('${uuid}')"><span class="material-symbols-outlined">download</span></label><button id="${uuid}" style="display:none"></button></div>`;
    }

    // Create a new row
}

//#endregion

function getAdapterTypeFromCutadapt() {
    var adapterSelect = document.getElementById("adapter-select");
    var adapterSelectID = adapterSelect.selectedIndex;
    var adapterType = adapterSelect.options[adapterSelectID].value;
    var adapterContent = Array.from(
        document.querySelectorAll(".sequence-end-input>.checkbox-btn"),
    );
    var doubleChecked = 0;
    for (var i = 0; i < 2; i++) {
        var readEnd = adapterContent[i].querySelector(
            ".checkbox-label>div",
        ).textContent;
        var readEndContent = adapterContent[i].querySelector(
            ".checkbox-label>input",
        ).value;
        var readChecked =
            adapterContent[i].querySelector(".checkbox-input").checked;
        console.log(
            `readEnd:${readEnd}, readEndContent:${readEndContent}, readChecked:${readChecked}`,
        );
        if (readChecked === true) doubleChecked++;
    }
    if (doubleChecked == 2) adapterType = "Linked adapter";
    console.log(`adapterType:${adapterType}`);
}

function getToolSetting(toolSettingID, functionVector) {
    var baseClass = document.getElementById(toolSettingID);
    var toggleBoxSetting = Array.from(baseClass.querySelectorAll(".toggle-box"));
    var optionSetting = Array.from(baseClass.querySelectorAll(".options"));
    for (var i = 0; i < toggleBoxSetting.length; i++) {
        var toggleTitle =
            toggleBoxSetting[i].querySelector(".toggle-box-title").textContent;
        var toggleChecked =
            toggleBoxSetting[i].querySelector(".toggle>input").checked;
        console.log(`toggleTitle: ${toggleTitle}, toggleChecked: ${toggleChecked}`);
    }
    for (var i = 0; i < optionSetting.length; i++) {
        var optionTitle =
            optionSetting[i].querySelector(".option-title").textContent;
        var optionValue;
        if (optionSetting[i].querySelector(".option-select")) {
            optionSelect = optionSetting[i].querySelector(".option-select");
            optionValue = optionSelect.options[optionSelect.selectedIndex].value;
        } else optionValue = optionSetting[i].querySelector("input").value;
        console.log(`optionTitle: ${optionTitle}, optionValue: ${optionValue}`);
    }
    if (functionVector != null) {
        for (var i = 0; i < functionVector.length; i++) {
            functionVector[i]();
        }
    }
}

function settingsFitForJSON(settings) {
    var modifiedobject = {};
    var keys = Object.keys(settings);
    for (var i = 0; i < keys.length; i++) {
        var modifiedKey = keys[i].replace(/ /g, "_");
        modifiedKey = modifiedKey.replace(/-/g, "_");
        modifiedKey = modifiedKey.replace(/\//g, "_");
        var value = settings[keys[i]];
        if (modifiedKey.search("upload file]") >= 0) {
            var file_name = value.name;
            modifiedobject[modifiedKey] = file_name;
        } else modifiedobject[modifiedKey] = value;
    }
    return modifiedobject;
}

function generatePipelineStepObject(pipelineName, tool, settings, step, email) {
    var processSettings = {};
    var copySettings = copyObject(settings);
    //var modifiedSettings = settingsFitForJSON(settings);
    processSettings["pipeline_name"] = pipelineName;
    processSettings["tool"] = tool;
    processSettings["setting"] = copySettings;
    processSettings["step"] = step;
    processSettings["email"] = email;
    return processSettings;
}

function checkStateStyle(textContent, status, element) {
    var checkStatus;
    var checkWrap;
    switch (element) {
        case "pipeline": {
            checkStatus = document.getElementById("pipeline-check-status");
            checkWrap = document.getElementById("pipeline-check-wrap");
            break;
        }
        case "project": {
            checkStatus = document.getElementById("project-check-status");
            checkWrap = document.getElementById("project-check-wrap");
            break;
        }
    }

    checkStatus.textContent = textContent;
    switch (status) {
        case "error": {
            checkWrap.style.backgroundColor = "#ffa8a8";
            checkStatus.style.color = "#bc0000";
            break;
        }
        case "loading": {
            checkStatus.style.color = "black";
            break;
        }
        case "reset": {
            checkWrap.style.backgroundColor = "";
            checkStatus.style.color = "";
            break;
        }
        case "finish": {
            checkWrap.style.backgroundColor = "green";
            checkStatus.style.color = "white";
            break;
        }
    }
}

async function fetchAPI(URL, Method, Header, Body) {
    var result = {
        error: "",
        dataType: "json",
        data: null,
        fileName: "",
    };

    try {
        var response;
        if (Method == "GET") {
            response = await fetch(URL, {
                method: Method,
                headers: Header,
            });
        } else if (Method == "POST" && (URL.search("fileDownload")) < 0) {
            response = await fetch(URL, {
                method: Method,
                headers: Header,
                body: Body,
            });
        } else if (Method == "POST" && (URL.search("fileDownload")) >= 0) {
            response = await fetch(URL, {
                method: Method,
                headers: Header,
                body: Body,
                responseType: 'arraybuffer'

            });
        }

        if (!response.ok) {
            throw new Error("Error on Network: " + response.statusText);
        }
        var contentType = response.headers.get("content-type");
        if (contentType) {
            if (contentType.includes("application/json")) {
                result.data = await response.json().catch((error) => {
                    throw new Error("Error on parsing JSON:  " + error.message);
                });
            } else if (contentType.includes("octet-stream")) {
                result.data = await response.arrayBuffer();
                result.dataType = "arraybuffer";
                var contentDisposition = response.headers.get("Content-Disposition");
                var matches = /filename="?([^"]+)"?/.exec(contentDisposition);
                result.fileName = matches ? matches[1] : "";
            }
        }
    } catch (error) {
        result.error = error;
    }

    return result;
}

async function readFileAsync(file) {
    return new Promise((resolve, reject) => {
        const reader = new FileReader();

        reader.onload = function (event) {
            resolve(event.target.result);
        };

        reader.onerror = function (error) {
            reject(String(error));
        };

        reader.readAsArrayBuffer(file);
    });
}

async function sendPipelineToServer(object) {
    console.log("disabled button");

    object.disabled = true;
    var loader = document.getElementById("loader");
    loader.style.display = "none";
    checkStateStyle("", "reset", "pipeline");

    console.log("SendPipelineToSever start");
    var checkedTab = checkTabclicked("tabGroup1");
    console.log(`User select tab: ${checkedTab}`);
    var email = document.getElementById("user-account").textContent;
    var fileSendToServer = [];
    var pipelineInfo = {};
    var pipelineName = document.getElementById(
        "RNA-Seq-content-pipeline-name",
    ).value;

    if (pipelineName != "" && pipelineName != "Info_Loading_Faild") {
        pipelineInfo["pipeline_name"] = pipelineName;
        pipelineInfo["email"] = email;
    } else {
        checkStateStyle("Please fill pipeline name", "error", "pipeline");
        object.disabled = false;
        return;
    }

    var pipelineNameCheckheader = {
        "Content-type": "application/json",
    };
    var pipelineNameCheckBody = JSON.stringify(pipelineInfo);
    console.log("pipelineNameCheckBody: ", pipelineNameCheckBody);
    loader.style.display = "flex";
    checkStateStyle("pipeline_name duplication check...", "loading", "pipeline");
    var pipelineNameCheckResult = await fetchAPI(
        "/pipeline_check",
        "POST",
        pipelineNameCheckheader,
        pipelineNameCheckBody,
    );
    if (pipelineNameCheckResult["error"] != "") {
        checkStateStyle(pipelineNameCheckResult["error"], "error", "pipeline");
        loader.style.display = "none";
        object.disabled = false;
        return;
    }
    var responseMSG = pipelineNameCheckResult["data"]["message"];

    if (responseMSG.search("Pipeline Non-Existed") < 0) {
        checkStateStyle(responseMSG, "error", "pipeline");
        loader.style.display = "none";
        object.disabled = false;
        return;
    }

    loader.style.display = "none";
    checkStateStyle("", "reset", "pipeline");

    console.log("pipelineNameCheck suceed");
    console.log("process pipeline");

    var finalObject = [];
    switch (checkedTab) {
        case "RNA-Seq-tab": {
            var QCTool = checkTabclicked("qc-tools");
            var alignTool = checkTabclicked("align-tools");
            var QCToolSetting;
            var alignToolSetting;

            switch (QCTool) {
                case "fastp": {
                    QCToolSetting = returnRNASeqSetting("fastp", false);
                    console.log("original Settings: ", QCToolSetting);
                    //var modifiedSetting = settingsFitForJSON(QCToolSetting);
                    //console.log("modified Settings: ", modifiedSetting);
                    //var pipelineStep = generatePipelineStepObject(1, 'fastp', QCToolSetting);
                    var pipelineStep = generatePipelineStepObject(
                        pipelineName,
                        "fastp",
                        QCToolSetting,
                        1,
                        email,
                    );

                    finalObject.push(pipelineStep);

                    break;
                }
                case "cutadapt": {
                    QCToolSetting = returnRNASeqSetting("cutadapt", false);

                    console.log("original Setting: ", QCToolSetting);
                    // var modifiedSetting = settingsFitForJSON(QCToolSetting);
                    // console.log("modified Settings: ", modifiedSetting);
                    //var pipelineStep = generatePipelineStepObject(1, 'cutadapt', QCToolSetting);
                    var pipelineStep = generatePipelineStepObject(
                        pipelineName,
                        "cutadapt",
                        QCToolSetting,
                        1,
                        email,
                    );
                    finalObject.push(pipelineStep);

                    break;
                }
            }

            switch (alignTool) {
                case "kallisto": {
                    alignToolSetting = returnRNASeqSetting("kallisto", false);
                    if (
                        alignToolSetting.hasOwnProperty("Genome index num") &&
                        alignToolSetting.hasOwnProperty("Genome index upload file")
                    ) {
                        var indexSelected = alignToolSetting["Genome index num"];
                        var fileName = alignToolSetting["Genome index upload file"].name;
                        if (indexSelected == 0) {
                            if (fileName == undefined) {
                                checkStateStyle(
                                    "No uploaded file found in the kallisto index section.",
                                    "error",
                                    "pipeline",
                                );
                                object.disabled = false;
                                return;
                            } else {
                                fileSendToServer.push(
                                    alignToolSetting["Genome index upload file"],
                                );
                            }
                        }
                    }
                    console.log("original Setting: ", alignToolSetting);
                    // var modifiedSetting = settingsFitForJSON(alignToolSetting);
                    // console.log("modified Settings: ", modifiedSetting);
                    //var pipelineStep = generatePipelineStepObject(2, 'kallisto', alignToolSetting);
                    var pipelineStep = generatePipelineStepObject(
                        pipelineName,
                        "kallisto",
                        alignToolSetting,
                        2,
                        email,
                    );
                    finalObject.push(pipelineStep);
                    break;
                }
                case "subread": {
                    alignToolSetting = returnRNASeqSetting("subread", false);
                    console.log("original Setting: ", alignToolSetting);
                    // var modifiedSetting = settingsFitForJSON(alignToolSetting);
                    // console.log("modified Settings: ", modifiedSetting);

                    if (
                        alignToolSetting.hasOwnProperty("Genome index num") &&
                        alignToolSetting.hasOwnProperty("Genome index upload file")
                    ) {
                        var indexSelected = alignToolSetting["Genome index num"];
                        var fileName = alignToolSetting["Genome index upload file"].name;
                        if (indexSelected == 0) {
                            if (fileName == undefined) {
                                checkStateStyle(
                                    "No uploaded file found in the subread index section.",
                                    "error",
                                    "pipeline",
                                );
                                object.disabled = false;
                                return;
                            } else
                                fileSendToServer.push(
                                    alignToolSetting["Genome index upload file"],
                                );
                        }
                    }

                    if (
                        alignToolSetting.hasOwnProperty("GTF index num") &&
                        alignToolSetting.hasOwnProperty("GTF index upload file")
                    ) {
                        var indexSelected = alignToolSetting["GTF index num"];
                        var fileName = alignToolSetting["GTF index upload file"].name;
                        if (indexSelected == 0) {
                            if (fileName == undefined) {
                                checkStateStyle(
                                    "No uploaded file found in the subread gtf section.",
                                    "error",
                                    "pipeline",
                                );
                                object.disabled = false;
                                return;
                            } else
                                fileSendToServer.push(
                                    alignToolSetting["GTF index upload file"],
                                );
                        }
                    }

                    // var pipelineStep = generatePipelineStepObject(2, 'subread', alignToolSetting);
                    var pipelineStep = generatePipelineStepObject(
                        pipelineName,
                        "subread",
                        alignToolSetting,
                        2,
                        email,
                    );
                    finalObject.push(pipelineStep);
                }
            }

            break;
        }
        case "Custom-tab": {
            var pipelineName = document.getElementById(
                "custom-content-pipeline-name",
            ).value;
            if (pipelineName != "") console.log("Pipeline name: ", pipelineName);
            else pipelineInfo["Pipeline name"] = pipelineName;
            var toolTable = document.getElementById("tool-table");

            for (var i = 1; i < toolTable.rows.length; i++) {
                var row = toolTable.rows[i];
                console.log(row);
                var uuid = row.querySelector(".custom-setting-uuid").childNodes[0]
                    .nodeValue;
                console.log(uuid);
                var select = row.querySelector(".custom-tool-select");
                var selectedTool = select.value.split("_")[0];
                console.log("seletedTool: ", selectedTool);
                console.log(`row index[${i}] uuid: ${uuid}`);
                var settings = null;
                if (customSettingCollection.hasOwnProperty(uuid))
                    settings = customSettingCollection[uuid];
                if (settings == null) {
                    console.log("The settings isn't exsited");
                    console.log("sendPipelineToServer failed");
                    break;
                }
                switch (selectedTool) {
                    case "kallisto": {
                        if (
                            settings.hasOwnProperty("Genome index num") &&
                            settings.hasOwnProperty("Genome index upload file")
                        ) {
                            var indexSelected = settings["Genome index num"];
                            var fileName = settings["Genome index upload file"].name;
                            if (indexSelected == 0) {
                                if (fileName == undefined) {
                                    checkStateStyle(
                                        "No uploaded file found in the kallisto index section.",
                                        "error",
                                        "pipeline",
                                    );
                                    object.disabled = false;
                                    return;
                                }
                            } else
                                fileSendToServer.push(settings["Genome index upload file"]);
                        }

                        break;
                    }
                    case "subread": {
                        if (
                            settings.hasOwnProperty("Genome index num") &&
                            settings.hasOwnProperty("Genome index upload file")
                        ) {
                            var indexSelected = settings["Genome index num"];
                            var fileName = settings["Genome index upload file"].name;
                            if (indexSelected == 0) {
                                if (fileName == undefined) {
                                    checkStateStyle(
                                        "No uploaded file found in the subread index section.",
                                        "error",
                                        "pipeline",
                                    );
                                    object.disabled = false;
                                    return;
                                } else
                                    fileSendToServer.push(settings["Genome index upload file"]);
                            }
                        }

                        if (
                            settings.hasOwnProperty("GTF index num") &&
                            settings.hasOwnProperty("GTF index upload file")
                        ) {
                            var indexSelected = settings["GTF index num"];
                            var fileName = settings["GTF index upload file"].name;
                            if (indexSelected == undefined) {
                                if (fileName == "") {
                                    checkStateStyle(
                                        "No uploaded file found in the subread gtf section.",
                                        "error",
                                        "pipeline",
                                    );
                                    object.disabled = false;
                                    return;
                                } else fileSendToServer.push(settings["GTF index upload file"]);
                            }
                        }

                        break;
                    }
                    case "fastp": {
                        break;
                    }
                    case "cutadapt": {
                    }
                }
                var processOrder = i;
                var pipelineStep = generatePipelineStepObject(
                    pipelineName,
                    selectedTool,
                    settings,
                    processOrder,
                    email,
                );
                finalObject.push(pipelineStep);
            }
            break;
        }
    }

    if (finalObject[0].hasOwnProperty("step")) {
        var processOrder = finalObject[0]["step"];
        console.log(processOrder);
        if (processOrder != 1) console.log("generate finalObject failed");
        else console.log(finalObject);
    }

    console.log("file sending process");

    if (fileSendToServer.length > 0) {
        loader.style.display = "flex";

        for (var i = 0; i < fileSendToServer.length; i++) {
            var fileContent = await readFileAsync(fileSendToServer[i]);
            if (fileContent instanceof ArrayBuffer)
                console.log(`${fileSendToServer[i].name} read to ArrayBuffer succed`);
            else {
                checkStateStyle(
                    `${fileSendToServer[i].name} read failed`,
                    "error",
                    "pipeline",
                );
                loader.style.display = "none";
                return;
            }
            checkStateStyle(
                `uploading ${fileSendToServer[i].name} ....`,
                "loading",
                "pipeline",
            );
            var headers = {
                "Content-Type": "application/octet-stream",
                fileName: fileSendToServer[i].name,
                email: email,
                pipelineName: pipelineName,
            };
            var body = fileContent;
            var fileUploadResult = await fetchAPI(
                "/pipeline_fileUpload",
                "POST",
                headers,
                body,
            );
            if (fileUploadResult["error"] != "") {
                checkStateStyle(fileUploadResult["error"], "error", "pipeline");
                loader.style.display = "none";
                object.disabled = false;
                return;
            }
            var uploadMSG = fileUploadResult["data"]["message"];
            var uploadPath = "";
            if (uploadMSG.search("Upload succeed") < 0) {
                checkStateStyle(uploadMSG, "error", "pipeline");
                loader.style.display = "none";
                object.disable = false;
                return;
            } else {
                uploadPath = fileUploadResult["data"]["filePath"];
            }
            //覆寫上傳成功file相對應finalObject的[]的value
            updateFileLocation(finalObject, uploadPath);
        }
    }

    console.log(finalObject);
    var pipelineInsertHeader = {
        "Content-type": "application/json",
    };
    var pipelineInsertBody = JSON.stringify(finalObject);
    var finalObjectResult = await fetchAPI(
        "/pipeline_insert",
        "POST",
        pipelineInsertHeader,
        pipelineInsertBody,
    );
    if (finalObjectResult["error"] != "") {
        checkStateStyle(finalObjectResult["error"], "error", "pipeline");
        loader.style.display = "none";
        object.disabled = false;
        return;
    }
    var pieplineMSG = finalObjectResult["data"]["message"];
    if (pieplineMSG.search("Insertion succeed") < 0) {
        checkStateStyle(uploadMSG, "error", "pipeline");
        loader.style.display = "none";
        object.disable = false;
        return;
    }

    console.log("enable button");

    object.disabled = false;

    loader.style.display = "none";
    checkStateStyle("Pipeline construction succeeded", "finish", "pipeline");
}

async function syncWithAccount(syncID, tool, type) {
    var selectElement = document.getElementById(syncID);
    var email = document.getElementById("user-account").textContent;
    if (email.search("@") < 0) return;

    var optionMap = {};
    for (var i = 0; i < selectElement.options.length; i++) {
        optionMap[selectElement.options[i].value] = true;
    }

    var header = {
        "Content-type": "application/json",
    };

    var body = JSON.stringify({
        email: email,
        tool: tool,
        type: type
    });
    var response = await fetchAPI("/pipeline_sync", "POST", header, body);
    var responseMSG = "";
    var optionAdd = [];
    if (response.error == "") {
        if (response.data.hasOwnProperty("message")) {
            responseMSG = response.data["message"];
        } else if (response.data.hasOwnProperty("optionArray")) {
            var optionArray = response.data["optionArray"];
            optionArray.forEach((item) => {
                if (!optionMap.hasOwnProperty(item)) optionAdd.push(item);
            });
        }
    }

    if (optionAdd.length > 0) {
        for (var i = 0; i < optionAdd.length; i++) {
            var newOption = document.createElement("option");
            newOption.value = optionAdd[i];
            newOption.text = optionAdd[i];
            selectElement.appendChild(newOption);
        }
    }
    if (responseMSG != "") console.log(responseMSG);
}

function updateFileLocation(finalObject, uploadPath) {
    for (var i = 0; i < finalObject.length; i++) {
        var pipelineObjectTool = finalObject[i]["tool"];
        if (
            pipelineObjectTool.search("kallisto") < 0 &&
            pipelineObjectTool.search("subread") < 0
        )
            continue;
        var pipelineObjectSetting = finalObject[i]["setting"];
        var fileName = "";
        var lastDotPos = 0;
        if (pipelineObjectSetting.hasOwnProperty("Genome index upload file")) {
            if (
                typeof pipelineObjectSetting["Genome index upload file"] === "string"
            ) {
                console.log("already replace");
            } else {
                fileName = pipelineObjectSetting["Genome index upload file"].name;
                if (fileName != "") {
                    lastDotPos = fileName.search("\\.");
                    console.log(fileName);
                    if (lastDotPos > 0) ;
                    {
                        fileName = fileName.substring(0, lastDotPos);
                        if (uploadPath.search(fileName) >= 0) {
                            finalObject[i]["setting"]["Genome index upload file"] =
                                uploadPath;
                        }
                    }
                }
            }
        }

        if (pipelineObjectSetting.hasOwnProperty("GTF index upload file")) {
            if (typeof pipelineObjectSetting["GTF index upload file"] === "string") {
                console.log("already replace");
            } else {
                fileName = pipelineObjectSetting["GTF index upload file"].name;
                if (fileName != "") {
                    console.log(fileName);
                    lastDotPos = fileName.search("\\.");
                    if (lastDotPos > 0) {
                        fileName = fileName.substring(0, lastDotPos);
                        if (uploadPath.search(fileName) >= 0)
                            finalObject[i]["setting"]["GTF index upload file"] = uploadPath;
                    }
                }
            }
        }
    }
}
