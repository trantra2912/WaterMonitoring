'use strict';
const {
  Model
} = require('sequelize');
module.exports = (sequelize, DataTypes) => {
  class Pwm extends Model {
    /**
     * Helper method for defining associations.
     * This method is not a part of Sequelize lifecycle.
     * The `models/index` file will call this method automatically.
     */
    static associate(models) {
      // define association here
    }
  }
  Pwm.init({
    Nhietdo: { allowNull: true, type: DataTypes.FLOAT },
    NTU: { allowNull: true, type: DataTypes.FLOAT },
    TDS: { allowNull: true, type: DataTypes.FLOAT },
    DO_baohoa: { allowNull: true, type: DataTypes.FLOAT },
    WQI_NTU: { allowNull: true, type: DataTypes.FLOAT },
  }, {
    sequelize,
    modelName: 'Pwm',
  });
  return Pwm;
};