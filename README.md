# Learning-Based Parkinson's Disease Monitoring System
### Hardware-Software Co-Design on Zynq-7000 SoC (ZedBoard)

## 📌 Project Overview
This project implements an end-to-end diagnostic pipeline to detect Parkinson's Disease through the analysis of spiral drawing patterns. By leveraging the **Zynq-7000 SoC**, the system combines a high-level Python-trained neural network with custom FPGA hardware acceleration to achieve high-speed, real-time inference.

---

## 🛠 Engineering Workflow
The repository is structured to reflect the complete development lifecycle, from model training to hardware deployment:

### 📂 [01_ML_Model_Python](./01_ML_Model_Python)
* **Objective**: Train the diagnostic "brain" using deep learning.
* **Activities**: 
    * Dataset preprocessing and normalization of spiral images to $128 \times 128$ resolution.
    * Convolutional Neural Network (CNN) construction using TensorFlow and Keras.
    * Exporting trained weights into a C++ header (`weights.h`) for hardware compatibility.

### 📂 [02_Vitis_HLS_IP](./02_Vitis_HLS_IP)
* **Objective**: Transform software logic into hardware RTL.
* **Activities**: 
    * Developing HLS-optimized C++ kernels to implement the CNN layers.
    * Generating a custom IP core to be used in the Vivado Block Design.

### 📂 [03_Vivado_Hardware](./03_Vivado_Hardware)
* **Objective**: SoC Architecture Design and Integration.
* **Activities**: 
    * Integrating the HLS IP with the Zynq Processing System (PS) via AXI4 interconnects.
    * Configuring hardware constraints and generating the bitstream for the ZedBoard.

### 📂 [04_Vitis_IDE_App](./04_Vitis_IDE_App)
* **Objective**: Embedded Software Deployment.
* **Activities**: 
    * Developing the final C application to manage data flow between the ARM processor and FPGA logic.
    * System verification using "Golden Logits" to ensure hardware-software parity.

---

## 🚀 Hardware & Tools
* **Development Board**: Avnet ZedBoard (Zynq-7000)
* **Camera Sensor**: OV7670 (connected via Pmod/GPIO)
* **Software Stack**: Vivado ML, Vitis HLS, Vitis IDE, and Google Colab (Python 3)

---


---

## 📊 Performance
The model utilizes optimized CNN layers to identify tremors with high precision. Detailed accuracy charts and loss curves are located in the [01_ML_Model_Python](./01_ML_Model_Python) folder.
