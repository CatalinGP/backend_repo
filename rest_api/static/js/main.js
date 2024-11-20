let otaStatusRequestRunning = false;
let otaStatusRequestIntervalId;

// Utility function to display JSON response in the output container
function displayResponse(data) {
    const responseContainer = document.getElementById('response-output');
    responseContainer.textContent = JSON.stringify(data, null, 2);
}

// Utility function to fetch logs and update the log table
function fetchLogs() {
    fetch('/api/logs')
        .then(response => response.json())
        .then(data => {
            const logBody = document.getElementById('log-body');
            logBody.innerHTML = '';

            data.logs.reverse().forEach((log, index) => {
                const row = document.createElement('tr');
                row.innerHTML = `<td>${index + 1}</td><td>${log}</td>`;
                logBody.appendChild(row);
            });
        })
        .catch(error => {
            console.error('Error fetching logs:', error);
        });
}

/**
 * Performs an API request using the Fetch API.
 * This method is a refactored version designed to avoid code repetition
 * by consolidating the fetch logic into a single function.
 * @param {string} url - The URL endpoint for the API request.
 * @param {string} method - The HTTP method for the request (e.g., 'GET', 'POST').
 * @param {Object|null} body - The body of the request, to be sent as JSON. Defaults to null if no body is provided.
 * @returns {Promise} - A promise that resolves to the JSON response from the server.
 */
function performApiRequest(url, method, body = null) {
    return fetch(url, {
        method: method,
        headers: {
            'Content-Type': 'application/json',
        },
        body: body ? JSON.stringify(body) : null,
    }).then(response => response.json())
      .then(data => {
          displayResponse(data);
          fetchLogs();
          return data;
      })
      .catch(error => {
          console.error('Error:', error);
      });
}

function sendFrame() {
    const canId = document.getElementById('can-id').value;
    const canData = document.getElementById('can-data').value;
    if (!canId || !canData) {
        alert('CAN ID and CAN Data cannot be empty.');
        return;
    }

    performApiRequest('/api/send_frame', 'POST', { can_id: canId, can_data: canData });
}

function requestIds() {
    performApiRequest('/api/request_ids', 'GET');
}

function updateToVersion() {
    let type, version;

    // Regular expression for validating file type (letters only)
    const typeRegex = /^[a-zA-Z]+$/;

    // Regular expression for validating version (numbers and dots only)
    const versionRegex = /^[0-9.]+$/;

    // Loop until valid file type is provided
    do {
        type = prompt('Enter file type (letters only):');
        if (type === null) {
            alert('Operation cancelled.');
            return;
        }
        if (!typeRegex.test(type)) {
            alert('Invalid file type. Please enter letters only.');
        }
    } while (!typeRegex.test(type));

    // Loop until valid version is provided
    do {
        version = prompt('Enter software version (numbers and dots only):');
        if (version === null) {
            alert('Operation cancelled.');
            return;
        }
        if (!versionRegex.test(version)) {
            alert('Invalid version. Please enter numbers and dots only.');
        }
    } while (!versionRegex.test(version));

    // Loop until valid version is provided
    do {
        ecu_id = prompt('Enter ECU id (numbers only):');
        if (ecu_id === null) {
            alert('Operation cancelled.');
            return;
        }
        if (!versionRegex.test(ecu_id)) {
            alert('Invalid version. Please enter numbers only.');
        }
    } while (!versionRegex.test(ecu_id));

    // If validation passes, perform the API request
    performApiRequest('/api/update_to_version', 'POST', { update_file_type: type, update_file_version: version, ecu_id: ecu_id });
}

function readInfoBattery() {
    performApiRequest('/api/read_info_battery', 'GET');

}

function readInfoEngine() {
    performApiRequest('/api/read_info_engine', 'GET');
}


function gDriveReadData() {
    fetch('/api/drive_update_data', {
        method: 'GET',
    }).then(response => response.json())
      .then(data => {
          document.getElementById('response-output').textContent = JSON.stringify(data);
          fetchLogs();
      });
}

function readInfoDoors() {
    performApiRequest('/api/read_info_doors', 'GET');
}

function writeInfoDoors() {
    const data = {
        door: prompt('Enter Door Status (0: closed, 1: open):') || null,
        passenger: prompt('Enter Passenger Door Status (0: closed, 1: open):') || null,
        passenger_lock: prompt('Enter Passenger Lock Status (0: locked, 1: unlocked):') || null,
        driver: prompt('Enter Driver Door Status (0: closed, 1: open):') || null,
        ajar: prompt('Enter Ajar Warning Status (0: no warning, 1: warning):') || null
    };
    performApiRequest('/api/write_info_doors', 'POST', data);
}

function writeInfoBattery() {
    const data = {
        battery_level: prompt('Enter Battery Energy Level:') || null,
        voltage: prompt('Enter Battery Voltage:') || null,
        battery_state_of_charge: prompt('Enter Battery State of Charge:') || null,
        percentage: prompt('Enter Battery Percentage:') || null,
        // temperature: prompt('Enter Battery Temperature:') || null,
        // life_cycle: prompt('Enter Battery Life Cycle:') || null,
        // fully_charged: prompt('Enter Battery Fully Charged Status:') || null,
        // range_battery: prompt('Enter Battery Range:') || null,
        // charging_time: prompt('Enter Battery Charging Time:') || null,
        // device_consumption: prompt('Enter Device Consumption:') || null
    };
    performApiRequest('/api/write_info_battery', 'POST', data);
}

function writeInfoEngine() {
    const data = {
        engine_rpm: prompt('Enter Engine RPM:') || null,
        coolant_temperature: prompt('Enter Coolant Temperature:') || null,
        throttle_position: prompt('Enter Throttle Position:') || null,
        vehicle_speed: prompt('Enter Vehicle Speed:') || null,
        engine_load: prompt('Enter Engine Load:') || null,
        fuel_level: prompt('Enter Fuel Level:') || null,
        oil_temperature: prompt('Enter Oil Temperature:') || null,
        fuel_pressure: prompt('Enter Fuel Pressure:') || null,
        intake_air_temperature: prompt('Enter Intake Air Temperature:') || null
    };

    performApiRequest('/api/write_info_engine', 'POST', data);
}

function writeInfoHVAC() {
    const data = {
        mass_air_flow: prompt('Enter Mass Air Flow:') || null,
        ambient_air_temperature: prompt('Enter Ambient Air Temperature:') || null,
        cabin_temperature: prompt('Enter Cabin Temperature:') || null,
        cabin_temperature_driver_set: prompt('Enter Cabin Temperature Driver Set:') || null,
        fan_speed: prompt('Enter Fan Speed:') || null,
        hvac_modes: prompt('Enter HVAC Modes:') || null,
    };

    performApiRequest('/api/write_info_hvac', 'POST', data);
}

function changeSession() {
    const input = prompt('Enter sub-function code (1 for default session, 2 for programming session, 3 for extended session):');
    if (input === null) {
        alert('Operation cancelled.');
        return;
    };

    const sub_funct = parseInt(input, 10);
    if (sub_funct !== 1 && sub_funct !== 2 && sub_funct != 3) {
        alert('Invalid input. Please enter 1 or 2.');
        return;
    };

    performApiRequest('/api/change_session', 'POST', { sub_funct: sub_funct });
}

function authenticate() {
    performApiRequest('/api/authenticate', 'GET');
}
function read_dtc_info() {
    performApiRequest('/api/read_dtc_info', 'GET');
}

function clear_dtc_info() {
    performApiRequest('/api/clear_dtc_info', 'GET');
}

function get_tester_pres() {
    performApiRequest('/api/tester_present', 'GET');
}

function get_data_ids() {
    performApiRequest('/api/get_identifiers', 'GET');
}

function readTimingInfo() {
    // Show descriptions for valid options
    const descriptions = `
    Please enter a sub-function code:
    1 - Read P2_MAX_TIME_DEFAULT and P2_STAR_MAX_TIME_DEFAULT
    3 - Read p2_max_time and p2_star_max_time
    `;

    const input = prompt(descriptions);
    if (input === null) {
        alert('Operation cancelled.');
        return;
    }

    const sub_funct = parseInt(input, 10); // Convert input to an integer

    // Check if sub_funct is either 1 or 3
    if (sub_funct !== 1 && sub_funct !== 3) {
        alert('Invalid input. Please enter 1 or 3.');
        return;
    }

    // Call the API with the correct sub_function value
    performApiRequest('/api/read_access_timing', 'POST', { sub_funct: sub_funct });
}

function resetECU() {
    const type_reset = prompt('Enter type of reset (soft or hard):') || null;
    const ecu_id = prompt('Enter ECU ID (10 or 11):') || null;

    if (!['soft', 'hard'].includes(type_reset)) {
        alert('Invalid reset type. Please enter "soft" or "hard".');
        return;
    }

    if (!['10', '11'].includes(ecu_id)) {
        alert('Invalid ECU ID. Please enter "10" or "11".');
        return;
    }

    const data = {
        type_reset: type_reset,
        ecu_id: ecu_id,
    };

    performApiRequest('/api/reset_ecu', 'POST', data);
}

function writeTimingInfo() {
    const p2Max = prompt('Enter value for P2 Max Time:');
    if (p2Max === null) {
        alert('Operation cancelled.');
        return;
    }

    const p2StarMax = prompt('Enter value for P2 Star Max Time:');
    if (p2StarMax === null) {
        alert('Operation cancelled.');
        return;
    }

    const parsedP2Max = parseInt(p2Max, 10);
    const parsedP2StarMax = parseInt(p2StarMax, 10);

    if (isNaN(parsedP2Max) || isNaN(parsedP2StarMax)) {
        alert('Invalid input. Please enter numeric values.');
        return;
    }

    const data = {
        p2_max: parsedP2Max,
        p2_star_max: parsedP2StarMax
    };

    performApiRequest('/api/write_timing', 'POST', data);
}
function readInfoHvac() {
    performApiRequest('/api/read_info_hvac', 'GET');
}

function selectRoutineControl()
{
    const dropdown = document.getElementById('routine-dropdown');
    const inputsContainer = document.getElementById('dynamic-routine-inputs');
    const selectedRoutine = dropdown.value;
  
    inputsContainer.innerHTML = '';
  
    const routineInputs = {
        EraseDataRoutine: [
            { placeholder: 'Receiver', id: 'receiver-ecu', value: '0x10'},
            { placeholder: 'Address', id: 'address-erase', value: '0x0800'},
            { placeholder: 'Nr of bytes', id: 'size', value: '0x05'}
        ],
        InitialiseOTARoutine: [
            { placeholder: 'Target', id: 'target-ecu', value: '0x11'},
            { placeholder: 'Sw Version', id: 'software-number', value: '1.0'},
        ],
        VerifyDataRoutine: [
            { placeholder: 'Receiver', id: 'receiver-ecu', value: '0x10'}
        ],
        WriteToFileRoutine: [
            { placeholder: 'Receiver', id: 'receiver-ecu', value: '0x10'},
        ],
        RollbackRoutine: [
            { placeholder: 'Receiver', id: 'receiver-ecu', value: '0x10'},
        ],
        ActivateRoutine: [
            { placeholder: 'Receiver', id: 'receiver-ecu', value: '0x10'},
        ],
    };
  
    const inputs = routineInputs[selectedRoutine] || [];
  
    inputs.forEach((inputConfig) => {
        const inputGroup = document.createElement('div');
        inputGroup.style.display = 'flex';
        inputGroup.style.flexDirection = 'column';
        inputGroup.style.alignItems = 'center';
        inputGroup.style.marginRight = '10px';

        const label = document.createElement('label');
        label.htmlFor = inputConfig.id;
        label.textContent = inputConfig.placeholder;

        const input = document.createElement('input');
        input.type = 'text';
        input.className = 'form-control';
        input.placeholder = inputConfig.placeholder;
        input.id = inputConfig.id;
        input.style.maxWidth = '150px';
        input.style.marginLeft = '10px';
        input.value = inputConfig.value;
        input.required = true;

        inputGroup.appendChild(input);
        inputGroup.appendChild(label);
        inputsContainer.appendChild(inputGroup);

    });

    if(inputs.length > 0) {
        const submitButton = document.createElement('button');
        submitButton.id = 'submit-routine-btn'
        submitButton.type = 'button';
        submitButton.className = "btn btn-primary mx-2";
        submitButton.textContent = 'Request routine';
        submitButton.style.marginLeft = '10px';
        submitButton.style.borderRadius = '10%';

        inputsContainer.appendChild(submitButton);
        submitButton.addEventListener('click', () => {
            handleRequest(selectedRoutine, 'routine')
        })
    }
  }

  function selectOtaAction(){
    const dropdown = document.getElementById('ota-actions-dropdown');
    const inputsContainer = document.getElementById('dynamic-ota-inputs');
    const selectedAction = dropdown.value;
  
    inputsContainer.innerHTML = '';
  
    const otaActionsInputs = {
        UpdateSoftwareAction: [
            { placeholder: 'Target', id: 'target-ecu', value: '0x11'},
            { placeholder: 'Address', id: 'address-update', value: '0x0800'},
            { placeholder: 'Sw Version', id: 'software-version', value: '1.0'},
            { placeholder: 'File type', id: 'file-type', value: 'zip'}
        ],
        TransferDataAction: [
            { placeholder: 'Target', id: 'target-ecu', value: '0x11'},
            { placeholder: 'Address', id: 'address-transfer', value: '0x0800'},
            { placeholder: 'Data bytes', id: 'data', value: '0xae25f9'}
        ],
        SyncOtaStatus: [
            { placeholder: 'Target', id: 'target-ecu', value: '0x11'},
            { placeholder: 'Ota State', id: 'ota-state', value: '0x00'}
        ],
    };
  
    const inputs = otaActionsInputs[selectedAction] || [];
  
    inputs.forEach((inputConfig) => {
        const inputGroup = document.createElement('div');
        inputGroup.style.display = 'flex';
        inputGroup.style.flexDirection = 'column';
        inputGroup.style.alignItems = 'center';
        inputGroup.style.marginRight = '10px';

        const label = document.createElement('label');
        label.htmlFor = inputConfig.id;
        label.textContent = inputConfig.placeholder;

        const input = document.createElement('input');
        input.type = 'text';
        input.className = 'form-control';
        input.id = inputConfig.id;
        input.style.maxWidth = '150px';
        input.style.marginLeft = '10px';
        input.required = true,
        input.value = inputConfig.value;
        input.disabled = (inputConfig.value == 'zip');

        inputGroup.appendChild(input);
        inputGroup.appendChild(label);
        inputsContainer.appendChild(inputGroup);
    });

    if(inputs.length > 0) {
        const submitButton = document.createElement('button');
        submitButton.id = 'submit-action-btn'
        submitButton.type = 'button';
        submitButton.className = "btn btn-primary mx-2";
        submitButton.textContent = 'Request action';
        submitButton.style.marginLeft = '10px';
        submitButton.style.borderRadius = '10%';

        inputsContainer.appendChild(submitButton);
        submitButton.addEventListener('click', () => {
            handleRequest(selectedAction, 'ota-action')
        })
    }
  }
  /**
   * sw_file_type = data.get('update_file_type')
    sw_file_version = data.get('update_file_version')
    ecu_id = data.get('ecu_id')
   */
    function handleRequest(request, type){
        let formInputs;
        if(type == 'routine'){
            formInputs = document.querySelectorAll('#dynamic-routine-inputs input');
        }
        else if(type == 'ota-action'){
            formInputs = document.querySelectorAll('#dynamic-ota-inputs input');
        }
        else{
            return;
        }
        formInputs = Array.from(formInputs);
        
        dataForApiRequest = {};
        let inputsValid = true;
        const hexPattern = /^0x[0-9A-Fa-f]+$/;
        const validEcuIds = [0x10, 0x11, 0x12, 0x13, 0x14];
    
        formInputs.forEach((input) => {
            const hexValue = parseInt(input.value, 16);
            if(input.id != 'file-type' && input.id != 'software-version' && hexPattern.test(input.value) == false){
                inputsValid = false;
            }
            else{
                switch(input.id)
                {
                    case 'receiver-ecu':
                        {
                            /* Same as below */
                        }
                    case 'target-ecu':
                        {
                            if(validEcuIds.includes(hexValue) == false){
                                inputsValid = false;
                            }
                            break;
                        }
                    case 'software-version':
                        {
                            const versionRegex = /^(?:[1-9]|1[0-6])\.(?:[0-9]|1[0-5])$/;
                            if(versionRegex.test(input.value) == false){
                                inputsValid = false;
                            }
                            break;
                        }
                    case 'address-erase':
                        {
                            if(hexValue < 0x0800){
                                inputsValid = false;
                            }
                            break;
                        }
                    case 'address-update':
                        {
                            break;
                        }
                    case 'address-transfer':
                        {
                            break;
                        }
                    case 'data':
                        {
                            break;
                        }
                    case 'file-type':
                        {
                            break;
                        }
                    case 'size':
                        {
                            if(size <= 0){
                                inputsValid = false;
                            }
                            break;
                        }
                    case 'ota-state':
                        {
                            break;
                        }
                    default: break;
                }
            }

            if(inputsValid){
                dataForApiRequest[input.id] = input.value;
            }
            else{
                console.log("Invalid input");
                return;
            }
        })

        /* Call request */
        switch(request){
            /* Extra validations for specific endpoints here */
            case 'UpdateSoftwareAction':
                {
                    performApiRequest('/api/update_to_version', 'POST', { 
                        update_file_type: dataForApiRequest['file-type'], 
                        update_file_version: dataForApiRequest['software-version'], 
                        ecu_id: dataForApiRequest['target-ecu'] 
                    });
                    break;
                }
            case 'TransferDataAction':
                {
                    performApiRequest('/api/transfer_data_to_ecu', 'POST',{
                        ecu_id: dataForApiRequest['target-ecu'],
                        address: dataForApiRequest['address-transfer'],
                        data_bytes: dataForApiRequest['data']
                    });
                    break;
                }
            case 'SyncOtaStatus':
                {
                    performApiRequest('/api/sync_ota_status', 'POST', {
                        ecu_id: dataForApiRequest['target-ecu'],
                        ota_state: dataForApiRequest['ota-state']
                    });
                    break;
                }
            case 'InitialiseOTARoutine':
                {
                    if(hexValue == 0x10){
                        // performApiRequest();
                    }
                    else{
                        console.log("OTA can be initialised only for mcu as receiver.")
                    }
                    break;
                }
            case 'EraseDataRoutine':
                {
                    performApiRequest('/api/erase_memory', 'POST', {
                        ecu_id: dataForApiRequest['receiver-ecu'],
                        address: dataForApiRequest['address-erase'],
                        nrBytes: dataForApiRequest['size'],
                    });
                    break;
                }
            case 'VerifyDataRoutine':
                {
                    performApiRequest('/api/verify_software', 'POST', {
                        ecu_id: dataForApiRequest['receiver-ecu'],
                    });
                    break;
                }
            case 'WriteToFileRoutine':
                {
                    // performApiRequest();
                    break;
                }
            case 'RollbackRoutine':
                {
                    performApiRequest('/api/rollback_software', 'POST', {
                        ecu_id: dataForApiRequest['receiver-ecu'],
                    });
                    break;
                }
            case 'ActivateRoutine':
                {
                    performApiRequest('/api/activate_software', 'POST', {
                        ecu_id: dataForApiRequest['receiver-ecu'],
                    });
                    break;
                }
            default: break;
        }
        console.log("Endpoint called");
    }

    async function getOtaStatus(event){
        if(event == 'click'){
            const response = await performApiRequest('/api/ota_status', 'POST', {
                ecu_id: "0x10",
            });
            document.getElementById('ota-status-btn').innerHTML = `Ota State<br><span style="color: black;">${response.state}`;
        }
        else if(event == 'dblclick'){
            otaStatusRequestRunning ^= 1;
            if(otaStatusRequestRunning){
                otaStatusRequestIntervalId = setInterval(async() => {
                    const response = await performApiRequest('/api/ota_status', 'POST', {
                        ecu_id: "0x10"
                    });
                    document.getElementById('ota-status-btn').innerHTML = `Ota State<br><span style="color: black;">${response.state}`;
                }, 1000);
            }
            else{
                clearInterval(otaStatusRequestIntervalId);
                document.getElementById('ota-status-btn').innerHTML = `Ota State`;
            }
        }
    }
  document.addEventListener('DOMContentLoaded', () => {
    document.getElementById('routine-dropdown').selectedIndex = 0;
    document.getElementById('ota-actions-dropdown').selectedIndex = 0;
  });