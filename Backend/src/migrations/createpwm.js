'use strict';
/** @type {import('sequelize-cli').Migration} */
module.exports = {
  async up(queryInterface, Sequelize) {
    await queryInterface.createTable('Pwms', {
      id: {
        allowNull: false,
        autoIncrement: true,
        primaryKey: true,
        type: Sequelize.INTEGER
      },
      NhietDo: {
        allowNull: true,
        type: Sequelize.FLOAT
      },
      NTU: {
        allowNull: true,
        type: Sequelize.FLOAT
      },
      TDS: {
        allowNull: true,
        type: Sequelize.FLOAT
      },
      DO_baohoa: {
        allowNull: true,
        type: Sequelize.FLOAT
      },
      WQI_NTU: {
        allowNull: true,
        type: Sequelize.FLOAT
      },
      createdAt: {
        allowNull: false,
        type: Sequelize.DATE
      },
      updatedAt: {
        allowNull: false,
        type: Sequelize.DATE
      }
    });
  },
  async down(queryInterface, Sequelize) {
    await queryInterface.dropTable('Pwms');
  }
};