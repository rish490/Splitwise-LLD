# Splitwise - Low Level Design

A low-level design implementation of a Splitwise-style expense sharing system built to explore scalable backend architecture, clean object-oriented design, and maintainable financial workflow modeling.

This project focuses on designing modular and extensible components commonly used in real-world expense management and payment settlement systems.

---

## Problem Statement

Design a system that allows users to:

- Create groups
- Add shared expenses
- Split expenses among users
- Track balances between participants
- Simplify and settle debts
- Maintain transaction history

The system should be scalable, maintainable, and extensible for future enhancements.

---

## Features

- User management
- Group expense tracking
- Equal / exact / percentage expense splitting
- Balance calculation
- Debt settlement workflows
- Transaction history management
- Modular class structure
- Extensible expense models

---

## Design Goals

The system was designed with emphasis on:

- scalability
- maintainability
- extensibility
- separation of concerns
- reusable architecture
- clean object-oriented design

---

## Concepts Explored

- Low-Level Design (LLD)
- Object-Oriented Programming (OOP)
- SOLID Principles
- Financial workflow modeling
- Transaction lifecycle handling
- Service abstraction
- Domain-driven design thinking

---

## Core Entities

- User
- Group
- Expense
- Split
- Balance Sheet
- Transaction
- Settlement Manager

---

## Expense Split Types

### Equal Split
Expense divided equally among participants.

### Exact Split
Custom exact amount assigned to each participant.

### Percentage Split
Expense divided based on percentage contribution.

---

## Possible Real-World Extensions

- Payment gateway integration
- Multi-currency support
- Notification systems
- Expense categories & analytics
- Recurring expenses
- Fraud detection workflows
- Async settlement processing
- Distributed transaction handling

---

## Tech Stack

- C++
- Object-Oriented Programming
- Low-Level Design
- Design Patterns

---

## Repository Structure

```text
Splitwise-LLD/
├── models/
├── services/
├── managers/
├── main.cpp
└── README.md
