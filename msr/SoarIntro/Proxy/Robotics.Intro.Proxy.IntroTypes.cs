//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:2.0.50727.832
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

using Microsoft.Ccr.Core;
using Microsoft.Dss.Core;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.ServiceModel.Dssp;
using System;
using System.Collections.Generic;
using System.Xml.Serialization;
using W3C.Soap;
using compression = System.IO.Compression;
using constructor = Microsoft.Dss.Services.Constructor;
using contractmanager = Microsoft.Dss.Services.ContractManager;
using contractmodel = Microsoft.Dss.Services.ContractModel;
using io = System.IO;
using pxintro = Robotics.Intro.Proxy;
using reflection = System.Reflection;


namespace Robotics.Intro.Proxy
{
    
    
    /// <summary>
    /// Intro Contract
    /// </summary>
    [XmlTypeAttribute(IncludeInSchema=false)]
    public sealed class Contract
    {
        
        /// The Unique Contract Identifier for the Intro service
        public const String Identifier = "http://schemas.tempuri.org/2006/08/intro.html";
        
        /// The Dss Service dssModel Contract(s)
        public static List<contractmodel.ServiceSummary> ServiceModel()
        {
            contractmanager.ServiceSummaryLoader loader = new contractmanager.ServiceSummaryLoader();
            return loader.GetServiceSummaries(typeof(Contract).Assembly);

        }
        
        /// <summary>
        /// Creates an instance of the service associated with this contract
        /// </summary>
        /// <param name="contructorServicePort">Contractor Service that will create the instance</param>
        /// <param name="partners">Optional list of service partners for new service instance</param>
        /// <returns>Result PortSet for retrieving service creation response</returns>
        public static DsspResponsePort<CreateResponse> CreateService(constructor.ConstructorPort contructorServicePort, params PartnerType[] partners)
        {
            DsspResponsePort<CreateResponse> result = new DsspResponsePort<CreateResponse>();
            ServiceInfoType si = new ServiceInfoType(Contract.Identifier, null);
            if (partners != null)
            {
                si.PartnerList = new List<PartnerType>(partners);
            }
            Microsoft.Dss.Services.Constructor.Create create =
                new Microsoft.Dss.Services.Constructor.Create(si, result);
            contructorServicePort.Post(create);
            return result;

        }
        
        /// <summary>
        /// Creates an instance of the service associated with this contract
        /// </summary>
        /// <param name="contructorServicePort">Contractor Service that will create the instance</param>
        /// <returns>Result PortSet for retrieving service creation response</returns>
        public static DsspResponsePort<CreateResponse> CreateService(constructor.ConstructorPort contructorServicePort)
        {
            return Contract.CreateService(contructorServicePort, null);
        }
    }
    
    /// <summary>
    /// Intro State
    /// </summary>
    [DataContract()]
    [XmlRootAttribute("IntroState", Namespace="http://schemas.tempuri.org/2006/08/intro.html")]
    public class IntroState : ICloneable, IDssSerializable
    {
        
        private Boolean _motorOn;
        
        private Boolean _forwardMovesOnly;
        
        private Int32 _stopTimeout;
        
        private Int32 _backUpTimeout;
        
        private Int32 _turnTimeout;
        
        private Int32 _timeoutVariance;
        
        private Int32 _minimumDriveTimeout;
        
        private Double _minimumPower;
        
        private Double _maximumPower;
        
        private Double _backUpPower;
        
        private DateTime _nextTimestamp;
        
        private Int32 _lastBumperNum;
        
        private Int32 _insideBehavior;
        
        private Boolean _spawnDebugger;
        
        /// <summary>
        /// Motor On
        /// </summary>
        [DataMember()]
        public Boolean MotorOn
        {
            get
            {
                return this._motorOn;
            }
            set
            {
                this._motorOn = value;
            }
        }
        
        /// <summary>
        /// Forward Moves Only
        /// </summary>
        [DataMember()]
        public Boolean ForwardMovesOnly
        {
            get
            {
                return this._forwardMovesOnly;
            }
            set
            {
                this._forwardMovesOnly = value;
            }
        }
        
        /// <summary>
        /// Stop Timeout
        /// </summary>
        [DataMember()]
        public Int32 StopTimeout
        {
            get
            {
                return this._stopTimeout;
            }
            set
            {
                this._stopTimeout = value;
            }
        }
        
        /// <summary>
        /// Back Up Timeout
        /// </summary>
        [DataMember()]
        public Int32 BackUpTimeout
        {
            get
            {
                return this._backUpTimeout;
            }
            set
            {
                this._backUpTimeout = value;
            }
        }
        
        /// <summary>
        /// Turn Timeout
        /// </summary>
        [DataMember()]
        public Int32 TurnTimeout
        {
            get
            {
                return this._turnTimeout;
            }
            set
            {
                this._turnTimeout = value;
            }
        }
        
        /// <summary>
        /// Timeout Variance
        /// </summary>
        [DataMember()]
        public Int32 TimeoutVariance
        {
            get
            {
                return this._timeoutVariance;
            }
            set
            {
                this._timeoutVariance = value;
            }
        }
        
        /// <summary>
        /// Minimum Drive Timeout
        /// </summary>
        [DataMember()]
        public Int32 MinimumDriveTimeout
        {
            get
            {
                return this._minimumDriveTimeout;
            }
            set
            {
                this._minimumDriveTimeout = value;
            }
        }
        
        /// <summary>
        /// Minimum Power
        /// </summary>
        [DataMember()]
        public Double MinimumPower
        {
            get
            {
                return this._minimumPower;
            }
            set
            {
                this._minimumPower = value;
            }
        }
        
        /// <summary>
        /// Maximum Power
        /// </summary>
        [DataMember()]
        public Double MaximumPower
        {
            get
            {
                return this._maximumPower;
            }
            set
            {
                this._maximumPower = value;
            }
        }
        
        /// <summary>
        /// Back Up Power
        /// </summary>
        [DataMember()]
        public Double BackUpPower
        {
            get
            {
                return this._backUpPower;
            }
            set
            {
                this._backUpPower = value;
            }
        }
        
        /// <summary>
        /// Next Timestamp
        /// </summary>
        [DataMember()]
        public DateTime nextTimestamp
        {
            get
            {
                return this._nextTimestamp;
            }
            set
            {
                this._nextTimestamp = value;
            }
        }
        
        /// <summary>
        /// Last Bumper Num
        /// </summary>
        [DataMember()]
        public Int32 lastBumperNum
        {
            get
            {
                return this._lastBumperNum;
            }
            set
            {
                this._lastBumperNum = value;
            }
        }
        
        /// <summary>
        /// Inside Behavior
        /// </summary>
        [DataMember()]
        public Int32 insideBehavior
        {
            get
            {
                return this._insideBehavior;
            }
            set
            {
                this._insideBehavior = value;
            }
        }
        
        /// <summary>
        /// Spawn Debugger
        /// </summary>
        [DataMember()]
        public Boolean SpawnDebugger
        {
            get
            {
                return this._spawnDebugger;
            }
            set
            {
                this._spawnDebugger = value;
            }
        }
        
        /// <summary>
        /// Copy To Intro State
        /// </summary>
        public virtual void CopyTo(IDssSerializable target)
        {
            IntroState typedTarget = target as IntroState;

            if (typedTarget == null)
                throw new ArgumentException("CopyTo({0}) requires type {0}", this.GetType().FullName);
            typedTarget.MotorOn = this.MotorOn;
            typedTarget.ForwardMovesOnly = this.ForwardMovesOnly;
            typedTarget.StopTimeout = this.StopTimeout;
            typedTarget.BackUpTimeout = this.BackUpTimeout;
            typedTarget.TurnTimeout = this.TurnTimeout;
            typedTarget.TimeoutVariance = this.TimeoutVariance;
            typedTarget.MinimumDriveTimeout = this.MinimumDriveTimeout;
            typedTarget.MinimumPower = this.MinimumPower;
            typedTarget.MaximumPower = this.MaximumPower;
            typedTarget.BackUpPower = this.BackUpPower;
            typedTarget.nextTimestamp = this.nextTimestamp;
            typedTarget.lastBumperNum = this.lastBumperNum;
            typedTarget.insideBehavior = this.insideBehavior;
            typedTarget.SpawnDebugger = this.SpawnDebugger;
        }
        
        /// <summary>
        /// Clone Intro State
        /// </summary>
        public virtual object Clone()
        {
            IntroState target = new IntroState();

            target.MotorOn = this.MotorOn;
            target.ForwardMovesOnly = this.ForwardMovesOnly;
            target.StopTimeout = this.StopTimeout;
            target.BackUpTimeout = this.BackUpTimeout;
            target.TurnTimeout = this.TurnTimeout;
            target.TimeoutVariance = this.TimeoutVariance;
            target.MinimumDriveTimeout = this.MinimumDriveTimeout;
            target.MinimumPower = this.MinimumPower;
            target.MaximumPower = this.MaximumPower;
            target.BackUpPower = this.BackUpPower;
            target.nextTimestamp = this.nextTimestamp;
            target.lastBumperNum = this.lastBumperNum;
            target.insideBehavior = this.insideBehavior;
            target.SpawnDebugger = this.SpawnDebugger;
            return target;

        }
        
        /// <summary>
        /// Serialize Serialize
        /// </summary>
        public virtual void Serialize(System.IO.BinaryWriter writer)
        {
            writer.Write(MotorOn);

            writer.Write(ForwardMovesOnly);

            writer.Write(StopTimeout);

            writer.Write(BackUpTimeout);

            writer.Write(TurnTimeout);

            writer.Write(TimeoutVariance);

            writer.Write(MinimumDriveTimeout);

            writer.Write(MinimumPower);

            writer.Write(MaximumPower);

            writer.Write(BackUpPower);

            Microsoft.Dss.Services.Serializer.BinarySerializationHelper.SerializeDateTime(nextTimestamp, writer);

            writer.Write(lastBumperNum);

            writer.Write(insideBehavior);

            writer.Write(SpawnDebugger);

        }
        
        /// <summary>
        /// Deserialize Deserialize
        /// </summary>
        public virtual object Deserialize(System.IO.BinaryReader reader)
        {
            MotorOn = reader.ReadBoolean();

            ForwardMovesOnly = reader.ReadBoolean();

            StopTimeout = reader.ReadInt32();

            BackUpTimeout = reader.ReadInt32();

            TurnTimeout = reader.ReadInt32();

            TimeoutVariance = reader.ReadInt32();

            MinimumDriveTimeout = reader.ReadInt32();

            MinimumPower = reader.ReadDouble();

            MaximumPower = reader.ReadDouble();

            BackUpPower = reader.ReadDouble();

            nextTimestamp = Microsoft.Dss.Services.Serializer.BinarySerializationHelper.DeserializeDateTime(reader);

            lastBumperNum = reader.ReadInt32();

            insideBehavior = reader.ReadInt32();

            SpawnDebugger = reader.ReadBoolean();

            return this;

        }
    }
    
    /// <summary>
    /// Move States
    /// </summary>
    [DataContract()]
    [XmlRootAttribute("MoveStates", Namespace="http://schemas.tempuri.org/2006/08/intro.html")]
    public enum MoveStates
    {
        
        /// <summary>
        /// Stop
        /// </summary>
        Stop = 0,
        
        /// <summary>
        /// Turn
        /// </summary>
        Turn = 1,
        
        /// <summary>
        /// Move Straight
        /// </summary>
        MoveStraight = 2,
    }
    
    /// <summary>
    /// Intro Operations
    /// </summary>
    [ServicePort()]
    [XmlTypeAttribute(IncludeInSchema=false)]
    public class IntroOperations : PortSet<Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup, Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop, Get, Replace>
    {
        
        /// <summary>
        /// Required Lookup request body type
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.LookupResponse,Fault> DsspDefaultLookup()
        {
            Microsoft.Dss.ServiceModel.Dssp.LookupRequestType body = new Microsoft.Dss.ServiceModel.Dssp.LookupRequestType();
            Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup op = new Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup(body);
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Post Dssp Default Lookup and return the response port.
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.LookupResponse,Fault> DsspDefaultLookup(Microsoft.Dss.ServiceModel.Dssp.LookupRequestType body)
        {
            Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup op = new Microsoft.Dss.ServiceModel.Dssp.DsspDefaultLookup();
            op.Body = body ?? new Microsoft.Dss.ServiceModel.Dssp.LookupRequestType();
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// A request to drop the service.
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.DefaultDropResponseType,Fault> DsspDefaultDrop()
        {
            Microsoft.Dss.ServiceModel.Dssp.DropRequestType body = new Microsoft.Dss.ServiceModel.Dssp.DropRequestType();
            Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop op = new Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop(body);
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Post Dssp Default Drop and return the response port.
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.DefaultDropResponseType,Fault> DsspDefaultDrop(Microsoft.Dss.ServiceModel.Dssp.DropRequestType body)
        {
            Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop op = new Microsoft.Dss.ServiceModel.Dssp.DsspDefaultDrop();
            op.Body = body ?? new Microsoft.Dss.ServiceModel.Dssp.DropRequestType();
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Required Get body type
        /// </summary>
        public virtual PortSet<IntroState,Fault> Get()
        {
            Microsoft.Dss.ServiceModel.Dssp.GetRequestType body = new Microsoft.Dss.ServiceModel.Dssp.GetRequestType();
            Get op = new Get(body);
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Post Get and return the response port.
        /// </summary>
        public virtual PortSet<IntroState,Fault> Get(Microsoft.Dss.ServiceModel.Dssp.GetRequestType body)
        {
            Get op = new Get();
            op.Body = body ?? new Microsoft.Dss.ServiceModel.Dssp.GetRequestType();
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Intro State
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.DefaultReplaceResponseType,Fault> Replace()
        {
            IntroState body = new IntroState();
            Replace op = new Replace(body);
            this.Post(op);
            return op.ResponsePort;

        }
        
        /// <summary>
        /// Post Replace and return the response port.
        /// </summary>
        public virtual PortSet<Microsoft.Dss.ServiceModel.Dssp.DefaultReplaceResponseType,Fault> Replace(IntroState body)
        {
            Replace op = new Replace();
            op.Body = body ?? new IntroState();
            this.Post(op);
            return op.ResponsePort;

        }
    }
    
    /// <summary>
    /// Get
    /// </summary>
    [XmlTypeAttribute(IncludeInSchema=false)]
    public class Get : Microsoft.Dss.ServiceModel.Dssp.Get<Microsoft.Dss.ServiceModel.Dssp.GetRequestType, PortSet<IntroState, Fault>>
    {
        
        /// <summary>
        /// Get
        /// </summary>
        public Get()
        {
        }
        
        /// <summary>
        /// Get
        /// </summary>
        public Get(Microsoft.Dss.ServiceModel.Dssp.GetRequestType body) : 
                base(body)
        {
        }
        
        /// <summary>
        /// Get
        /// </summary>
        public Get(Microsoft.Dss.ServiceModel.Dssp.GetRequestType body, Microsoft.Ccr.Core.PortSet<IntroState,W3C.Soap.Fault> responsePort) : 
                base(body, responsePort)
        {
        }
    }
    
    /// <summary>
    /// Replace
    /// </summary>
    [XmlTypeAttribute(IncludeInSchema=false)]
    public class Replace : Microsoft.Dss.ServiceModel.Dssp.Replace<IntroState, PortSet<Microsoft.Dss.ServiceModel.Dssp.DefaultReplaceResponseType, Fault>>
    {
        
        /// <summary>
        /// Replace
        /// </summary>
        public Replace()
        {
        }
        
        /// <summary>
        /// Replace
        /// </summary>
        public Replace(IntroState body) : 
                base(body)
        {
        }
        
        /// <summary>
        /// Replace
        /// </summary>
        public Replace(IntroState body, Microsoft.Ccr.Core.PortSet<Microsoft.Dss.ServiceModel.Dssp.DefaultReplaceResponseType,W3C.Soap.Fault> responsePort) : 
                base(body, responsePort)
        {
        }
    }
}
