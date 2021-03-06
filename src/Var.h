#pragma once

#include "cinder/Json.h"
#include "cinder/Log.h"
#include "cinder/Utilities.h"

#include <map>

#include "Watchdog.h"

// Eric Renaud-Houde - Jan 2015
// Credit to Rich's live DartBag work.

namespace cinder {
	
	class JsonBag;
	class VarBase;
	template<typename T> class Var;
	
	JsonBag* bag();
	
	class JsonBag : public ci::Noncopyable {
	public:
		const fs::path& getFilepath() const { return mJsonFilePath; }
		void setFilepath( const fs::path& path );
		void save() const;
		void load();
		void unwatch();

		void setIsLive( bool live );
		bool getIsLive() const { return mIsLive; }

		int getVersion() const { return mVersion; }
		void setVersion( int version ) { mVersion = version; }
		bool isLoaded() const { return mIsLoaded; }

		const std::map<std::string, std::map<std::string, VarBase*>>& getItems() const { return mItems; }
	private:
		JsonBag();
		
		void emplace( VarBase* var, const std::string& name, const std::string groupName );
		void removeTarget( void* target );
		
		static std::unique_ptr<JsonBag>	mInstance;
		static std::once_flag			mOnceFlag;
		
		std::map<std::string, std::map<std::string, VarBase*>> mItems;
		ci::fs::path	mJsonFilePath;
		int				mVersion;
		bool			mIsLoaded;

		bool			mIsLive;
		
		friend JsonBag* ci::bag();
		friend class VarBase;
		template<typename T> friend class Var;
	};
	
	class VarBase {
	public:
		VarBase( void* target );
		virtual ~VarBase();
		
		void setOwner( JsonBag *owner ) { mOwner = owner; }
		
		void setUpdateFn( const std::function<void()> &updateFn, bool call = false );
		void callUpdateFn();
		
		void * getTarget() const { return mVoidPtr; }

		virtual bool draw( const std::string& name ) = 0;
		virtual void save( const std::string& name, ci::JsonTree* tree ) const = 0;
		virtual void load( ci::JsonTree::ConstIter& iter ) = 0;
	protected:
		std::function<void()>	mUpdateFn;

		JsonBag*	mOwner;
		void*		mVoidPtr;
	};
	
	template<typename T>
	class Var : public ci::Noncopyable, public VarBase {
	public:
		Var( const T& value, const std::string& name, const std::string& groupName = "default", float min = 0.0f, float max = 1.0f )
		: VarBase{ &mValue }
		, mValue( value )
		, mMin{ min }
		, mMax{ max }
		{
			ci::bag()->emplace( this, name, groupName );
		}
		virtual ~Var() { }

		virtual operator const T&() const { return mValue; }
		
		virtual Var<T>& operator=( T value )
		{
			update( value );
			return *this;
		}		
		virtual const T&	value() const { return mValue; }
		virtual const T&	operator()() const { return mValue; }
	protected:

		void update( T value ) {
			//if( mValue != value ) {
				mValue = value;
				callUpdateFn();
			//}
		}

#ifdef VAR_IMGUI
		virtual bool draw( const std::string& name ) override;
#else
		virtual bool draw( const std::string& name ) override { return false; }
#endif
		virtual void save( const std::string& name, ci::JsonTree* tree ) const override;
		virtual void load( ci::JsonTree::ConstIter& iter ) override;
	
		T						mValue;
		float					mMin, mMax;
		friend class JsonBag;
	};
} //namespace live

