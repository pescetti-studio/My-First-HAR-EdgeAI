%% 3D Tensor Generation
% This MATLAB script handles the pre-processing and organization of raw
% sensor data (accelerometer and gyroscope) into 3D tensors suitable for
% training a Long Short-Term Memory (LSTM) neural network.
% It also divides the dataset into training, validation, and test sets.

%% 1. Initialization and Preprocessing Parameters
% Resampling on a uniformly spaced time scale
% This comment indicates that one of the main goals is to standardize the sampling frequency.

% Path to the main raw dataset folder.
% 'pwd' returns the current working directory, from which the dataset is accessed.
basePath = fullfile(pwd, 'Raw_dataset');

% Key preprocessing parameters for data handling.
Fs = 50;                        % Desired sampling frequency (Hz) for the resampled data.
                                % This value is crucial for time series consistency.
windowSize = 2 * Fs;            % Time window size in samples.
                                % 2 seconds * 50 Hz = 100 samples per window.
stepSize = windowSize * 0.25;   % Sliding window step size.
                                % 0.25 indicates a 75% overlap between consecutive windows (100 * 0.25 = 25 samples).
                                % Overlap increases the number of training examples and better captures transitions.

% Initialization of variables that will store data and labels.
% These will be populated during the processing of all sessions.
attivitaUniche = {};            % Cell array to store unique activity names (e.g., 'Walking', 'Running').
X_train = [];                   % Matrix for training data (initially empty).
y_train = [];                   % Vector for training labels.
X_val = [];                     % Matrix for validation data.
y_val = [];                     % Vector for validation labels.
X_test = [];                    % Matrix for test data.
y_test = [];                    % Vector for test labels.

%% 2. Folder Scanning and Dataset Split Management
% Gets information about subfolders within the basePath.
% These subfolders represent different activities (e.g., "Downstairs", "Running").
activityFolders = dir(basePath);
% Filters the results to include only directories and exclude system folders (e.g., '.' and '..').
activityFolders = activityFolders([activityFolders.isdir] & ~startsWith({activityFolders.name}, '.'));

% Scans each activity folder.
for i = 1:length(activityFolders)
    activity = activityFolders(i).name;    % Current activity name.
    activityPath = fullfile(basePath, activity); % Full path to the activity folder.
    attivitaUniche{end+1} = activity;      % Adds the activity name to the unique activities list.
    label = i;                             % Assigns a numerical label (1, 2, 3...) to the current activity.

    % Gets subfolders within the activity folder.
    % These subfolders represent different acquisition sessions for that activity.
    sessionFolders = dir(activityPath);
    % Filters to include only valid directories.
    sessionFolders = sessionFolders([sessionFolders.isdir] & ~startsWith({sessionFolders.name}, '.'));

    % Extracts only the session names.
    sessionNames = {sessionFolders.name};
    numSessions = numel(sessionNames); % Total number of sessions for this activity.

    % Generates a random permutation index to shuffle sessions.
    idx = randperm(numSessions);

    % Calculates the split of sessions into training (70%), validation (20%), and test (10%).
    % This is a "subject-based split" or "session-based split" to prevent data leakage.
    nTrain = round(0.7 * numSessions); % Number of sessions for training.
    nVal   = round(0.2 * numSessions); % Number of sessions for validation.
    nTest  = numSessions - nTrain - nVal; % Number of sessions for testing (the remainder).

    % Assigns session names to their respective sets.
    trainNames = sessionNames(idx(1:nTrain));
    valNames   = sessionNames(idx(nTrain+1:nTrain+nVal));
    testNames  = sessionNames(idx(nTrain+nVal+1:end));

    %% 3. Data Processing for Each Session
    % Iterates through all sessions within the current activity.
    for j = 1:numSessions
        sessionName = sessionNames{j};          % Current session name.
        sessionPath = fullfile(activityPath, sessionName); % Full path to the session.

        % Defines the data file paths for accelerometer and gyroscope.
        accFile = fullfile(sessionPath, 'accelerometer.txt');
        gyroFile = fullfile(sessionPath, 'gyroscope.txt');

        % Checks if both data files exist for the current session.
        if exist(accFile, 'file') && exist(gyroFile, 'file')
            % Reads data from the files. 'readmatrix' is a convenient function for importing tabular data.
            accData = readmatrix(accFile);
            gyroData = readmatrix(gyroFile);

            % Extracts time columns (first column, converted from ms to seconds) and XYZ data.
            tAcc = accData(:,1) / 1000;  % Time in seconds for accelerometer.
            accXYZ = accData(:,2:4);     % XYZ data for accelerometer.
            tGyro = gyroData(:,1) / 1000;% Time in seconds for gyroscope.
            gyroXYZ = gyroData(:,2:4);   % XYZ data for gyroscope.

            % Skips sessions with insufficient data (fewer than 10 samples).
            if size(accXYZ,1) < 10 || size(gyroXYZ,1) < 10
                continue % Moves to the next iteration (next session).
            end

            % Calculates the common time interval between the two sensors for synchronization.
            t0 = max(min(tAcc), min(tGyro)); % Maximum start time among the minimums.
            tN = min(max(tAcc), max(tGyro)); % Minimum end time among the maximums.

            % Skips the session if the common time interval is too short for a 2-second window.
            if (tN - t0) < 2
                continue % Moves to the next iteration.
            end

            % Generates a uniformly spaced time vector for resampling.
            % Resampling is crucial to have time series with a constant frequency.
            tUniform = linspace(0, tN - t0, round((tN - t0) * Fs))';

            % Shifts time instants to start from 0 for the current session.
            tAcc = tAcc - t0;
            tGyro = tGyro - t0;

            % Handles duplicate times in original data (if any) for spline interpolation.
            % 'unique' ensures that time instants are strictly increasing.
            [tAcc, ia] = unique(tAcc); accXYZ = accXYZ(ia,:);
            [tGyro, ig] = unique(tGyro); gyroXYZ = gyroXYZ(ig,:);

            % Resamples XYZ data using spline interpolation.
            % 'spline' is an interpolation method that creates smoother curves.
            accInterp = spline(tAcc, accXYZ', tUniform)';
            gyroInterp = spline(tGyro, gyroXYZ', tUniform)';

            % Combines the resampled accelerometer and gyroscope data into a single matrix.
            % allData will have dimensions (NumSamples x 6), where 6 are the features (3 acc, 3 gyro).
            allData = [accInterp gyroInterp];  % Example: (N x 6)

            %% 4. Window Segmentation and Set Assignment
            % Iterates over 'allData' with a sliding window to create segments.
            % This loop extracts 2-second windows with a 75% overlap.
            for k = 1:stepSize:(size(allData,1) - windowSize + 1)
                segment = allData(k:k+windowSize-1, :);  % Extracts a data segment (e.g., 100 x 6).

                % Assigns the segment to the training, validation, or test set.
                % The 'ismember' check verifies which set (train/val/test) the current session belongs to.
                if ismember(sessionName, trainNames)
                    X_train = cat(3, X_train, segment); % Appends the segment to X_train along the third dimension.
                    y_train(end+1,1) = label;          % Appends the corresponding label.
                elseif ismember(sessionName, valNames)
                    X_val = cat(3, X_val, segment);     % Appends the segment to X_val.
                    y_val(end+1,1) = label;
                elseif ismember(sessionName, testNames)
                    X_test = cat(3, X_test, segment);   % Appends the segment to X_test.
                    y_test(end+1,1) = label;
                end
            end
        end
    end
end

%% 5. Tensor Reshaping and Saving
% Reorders the dimensions of the tensors.
% LSTM networks in frameworks like Keras/TensorFlow expect a format of [NumExamples x TimeSteps x Features].
% 'permute' reorganizes dimensions from (TimeSteps x Features x NumExamples) to (NumExamples x TimeSteps x Features).
X_train = permute(X_train, [3 1 2]);
X_val   = permute(X_val,   [3 1 2]);
X_test  = permute(X_test,  [3 1 2]);

% Prints the final dimensions of the tensors for verification.
% This is useful to ensure the data has the correct shape before being passed to the model.
fprintf("Train: X [%d x %d x %d], y [%d]\n", size(X_train,1), size(X_train,2), size(X_train,3), length(y_train));
fprintf("Val:   X [%d x %d x %d], y [%d]\n", size(X_val,1),   size(X_val,2),   size(X_val,3),   length(y_val));
fprintf("Test:  X [%d x %d x %d], y [%d]\n", size(X_test,1),  size(X_test,2),  size(X_test,3),  length(y_test));

% Saves the final tensors and activity names to a .mat file.
% This file can be easily loaded in MATLAB or, with appropriate libraries, also in Python.
save('dataset_split.mat', ...
     'X_train', 'y_train', ...
     'X_val',   'y_val', ...
     'X_test',  'y_test', ...
     'attivitaUniche');