# My-First-HAR-Edge-AI
Simple LSTM neural network for Human Activity Recognition (Downstairs, Upstairs, Walking, Running, Standing) from accelerometer/gyroscope data. Implemented on STM32 Nucleo F401RE for real-time Edge AI. Includes all MATLAB, Colab, and C code.
For more in-depth information and technical details, please refer to our [Medium article](https://medium.com/@crocilorenzo01/my-first-har-ai-from-dataset-to-microcontroller-3b6c45d7fcf5).
_________________________________________________________________________________________________

## Getting Started: A Step-by-Step Tutorial
Follow these steps to set up the project, train your model, and deploy it to your STM32 microcontroller.

### Step 1: Download the public dataset
First, you'll need to download the public Human Activity Recognition dataset. This dataset contains sensor data (accelerometer, gyroscope, and magnetometer). For this project, we'll only be using the accelerometer and gyroscope data. Download the [Dataset](https://data.mendeley.com/datasets/xknhpz5t96/2).

### Step 2: Prepare data with MATLAB
This step involves pre-processing the raw sensor data and converting it into 3D tensors suitable for training the LSTM model.
1. Open MATLAB
2. Navigate to and open the gen_tensor.m file.
3. Crucially, modify the basePath variable within gen_tensor.m to point to the directory where you downloaded and unzipped your dataset.
4. Run the gen_tensor.m script. This script will process the data and save a file named dataset_split.mat in your working directory. This .mat file contains the prepared training, validation, and test datasets.

### Step 3: Train the LSTM model on Google Colab
This part involves training your Human Activity Recognition (HAR) model using the prepared data.
1. Open the HAR_lstm_STM32CubeAI.ipynb notebook on Google Colab. You can directly open it by going to Colab, clicking "File" -> "Open notebook", and then selecting "GitHub" and pasting the URL of this notebook from your repository.
2. Execute all code cells sequentially within the Colab notebook. This notebook will load the dataset_split.mat file, define and train the LSTM model, and then convert it into a format optimized for embedded systems.

Upon successful execution, the notebook will generate two essential files: model_weights.c and model_weights.h. These files contain the learned parameters of your neural network in C array format.

### Step 4: Integrate Model Files into the STM32 Project
Now, you'll copy the generated model files into the STM32CubeIDE project structure.
1. Locate the generated model_weights.c and model_weights.h files (these are typically downloaded from Colab).
2. Copy model_weights.c into the HAR-EdgeAI/Core/Src directory of your STM32 project.
3. Copy model_weights.h into the HAR-EdgeAI/Core/Inc directory of your STM32 project.

### Step 5: Build and Run on STM32 Microcontroller
This final step involves compiling the project and deploying it to your hardware.
1. Open the STM32CubeIDE project.
2. Connect your hardware:
  - Connect the STM32 Nucleo-F401RE microcontroller board to your computer via USB.
  - Ensure the X-NUCLEO-IKS01A3 expansion board (which integrates the accelerometer and gyroscope) is correctly mounted on the Nucleo board.

3. Build and Run the project in STM32CubeIDE (usually by clicking the "Run" or "Debug" button). This will compile the C code, including your integrated model, and flash it onto the microcontroller.

### Step 6: Monitor Real-Time Predictions
Once the firmware is running on your STM32, you can observe the real-time activity predictions.
1. Open a serial terminal program (e.g., PuTTY, Tera Term, RealTerm, or the built-in serial monitor in STM32CubeIDE).
2. Configure the serial terminal with a Baud Rate of 115200.
3. You should now see real-time classifications of human activities (e.g., "Walking", "Running", "Standing") and a confidence percentage printed to the terminal, based on the sensor data acquired by the STM32 board.

