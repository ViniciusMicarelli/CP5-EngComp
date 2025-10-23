# 🧠 CP2 – Sistema de Dados Robusto (FreeRTOS + ESP32)

> **Disciplina:** Sistemas de Tempo Real  
> **Curso:** Engenharia da Computação  
> **Aluno:** **Vinícius Ribeiro Micarelli**  
> **RM:** **86868**

---

## 📋 Descrição do Projeto

Este projeto implementa um **sistema multitarefa robusto com FreeRTOS**, utilizando o **ESP32**.  
O sistema é dividido em **três módulos principais**, cada um executado em uma *task* independente, além de possuir **mecanismos de supervisão e segurança (WDT)** para garantir o funcionamento confiável mesmo sob falhas.

O objetivo é demonstrar **robustez, paralelismo e tratamento de falhas**, simulando um ambiente real de sistemas embarcados críticos.

---

## ⚙️ Estrutura de Módulos

### 🧩 Módulo 1 – Geração de Dados
- Produz valores inteiros sequenciais e pseudoaleatórios.  
- Envia cada dado para a **fila de comunicação (`Queue`)**.  
- Caso a fila esteja cheia, o dado é descartado.  
- Alimenta o **Watchdog Timer (WDT)**.

📘 **Logs Exemplo:**
![logs](https://github.com/user-attachments/assets/9c2ec796-c0b7-4c18-b3c4-52931bcd8673)
